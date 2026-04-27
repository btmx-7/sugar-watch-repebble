#!/usr/bin/env node
/**
 * Steady Dev Hub — Local API server
 * Runs pebble CLI commands, streams output via SSE, manages dev-config.json
 */

const express = require('express');
const { spawn }  = require('child_process');
const fs         = require('fs');
const path       = require('path');

const app          = express();
const PORT         = 3333;
const PROJECT_ROOT = path.resolve(__dirname, '..');
const CONFIG_FILE  = path.join(PROJECT_ROOT, 'dev-config.json');

const DEFAULT_CONFIG = {
  platform:    'emery',
  demoMode:    false,
  demoState:   2,
  layout:      0,
  slots:       [2, 1, 5, 3],
  units:       'mgdl',
  thresholds:  { urgentLow: 55, low: 70, high: 180, urgentHigh: 250 },
  graphWindow: 25,
};

// ── Config helpers ─────────────────────────────────────────────────────────

function readConfig() {
  try {
    return { ...DEFAULT_CONFIG, ...JSON.parse(fs.readFileSync(CONFIG_FILE, 'utf8')) };
  } catch {
    return { ...DEFAULT_CONFIG };
  }
}

function writeConfig(data) {
  const merged = { ...readConfig(), ...data };
  fs.writeFileSync(CONFIG_FILE, JSON.stringify(merged, null, 2) + '\n', 'utf8');
}

// ── State ──────────────────────────────────────────────────────────────────

const sseClients        = new Set();
let   currentProc       = null;
let   isBuilding        = false;
let   lastBuiltConfig   = null;   // config used in last successful build
let   lastInstalledConfig = null; // config used in last successful install

// ── SSE ────────────────────────────────────────────────────────────────────

function broadcast(event, data) {
  const msg = `event: ${event}\ndata: ${JSON.stringify(data)}\n\n`;
  sseClients.forEach(c => { try { c.write(msg); } catch {} });
}

// ── Express ────────────────────────────────────────────────────────────────

app.use(express.json());
app.use(express.static(__dirname));

app.get('/api/ping', (_req, res) => res.json({
  ok: true,
  building:      isBuilding,
  lastBuilt:     lastBuiltConfig,
  lastInstalled: lastInstalledConfig,
}));

app.get('/api/config', (_req, res) => res.json(readConfig()));

app.post('/api/config', (req, res) => {
  writeConfig(req.body);
  res.json({ ok: true });
});

app.get('/api/events', (req, res) => {
  res.writeHead(200, {
    'Content-Type':      'text/event-stream',
    'Cache-Control':     'no-cache',
    'Connection':        'keep-alive',
    'X-Accel-Buffering': 'no',
  });
  // Send current state immediately on connect
  res.write(`event: ping\ndata: ${JSON.stringify({
    building:      isBuilding,
    lastBuilt:     lastBuiltConfig,
    lastInstalled: lastInstalledConfig,
  })}\n\n`);
  sseClients.add(res);
  req.on('close', () => sseClients.delete(res));
});

// ── Darwin compiler env ────────────────────────────────────────────────────

function darwinCompilerEnv() {
  if (process.platform !== 'darwin') return {};
  // Belt-and-suspenders alongside the wscript fix: inject CC/CXX so every
  // pebble build spawned by the hub works even if $PATH lacks gcc.
  const env = {};
  const which = cmd => { try { require('child_process').execSync(`which ${cmd}`, { stdio: 'pipe' }); return true; } catch { return false; } };
  if (!which('gcc'))  env.CC  = '/usr/bin/clang';
  if (!which('g++'))  env.CXX = '/usr/bin/clang++';
  return env;
}

// ── Command runner ─────────────────────────────────────────────────────────

function buildCmdEnv(cfg) {
  const env = { ...darwinCompilerEnv() };
  if (cfg.demoMode) {
    env.DEMO_DATA  = '1';
    env.DEMO_STATE = String(cfg.demoState);
  }
  return env;
}

function envPrefix(env) {
  // Exclude CC/CXX from display — they're transparent plumbing
  const display = Object.entries(env)
    .filter(([k]) => !['CC','CXX'].includes(k))
    .map(([k, v]) => `${k}=${v}`)
    .join(' ');
  return display;
}

/**
 * Run steps sequentially, stop on first non-zero exit.
 * @param {Array<{cmd,args,env?,label?}>} steps
 * @param {Function?} onSuccess  called with no args after all steps succeed
 */
function runSequence(steps, onSuccess) {
  if (isBuilding) {
    broadcast('info', { text: 'Already running — kill it first.\n' });
    return;
  }
  isBuilding = true;
  broadcast('building', { value: true });
  broadcast('clear', {});

  let stepIdx = 0;

  function next() {
    if (stepIdx >= steps.length) { finish(0); return; }

    const { cmd, args, env = {}, label = '' } = steps[stepIdx++];
    const mergedEnv  = { ...process.env, ...env };
    const prefix     = envPrefix(env);
    const displayCmd = (prefix ? prefix + ' ' : '') + [cmd, ...args].join(' ');

    broadcast('cmd', { text: `$ ${displayCmd}${label ? '  # ' + label : ''}` });

    currentProc = spawn(cmd, args, { cwd: PROJECT_ROOT, env: mergedEnv });

    currentProc.stdout.on('data', d => broadcast('stdout', { text: d.toString() }));
    currentProc.stderr.on('data', d => broadcast('stderr', { text: d.toString() }));

    currentProc.on('error', err => {
      const hint = err.code === 'ENOENT'
        ? 'pebble not found — is the Pebble SDK on PATH?\n'
        : err.message + '\n';
      broadcast('stderr', { text: hint });
      finish(1);
    });

    currentProc.on('close', code => {
      currentProc = null;
      if (code === 0) {
        if (stepIdx < steps.length) broadcast('info', { text: `✓ Step ${stepIdx}/${steps.length} done\n` });
        next();
      } else {
        finish(code);
      }
    });
  }

  function finish(code) {
    currentProc = null;
    isBuilding  = false;
    if (code === 0 && onSuccess) onSuccess();
    broadcast('done',     { code, success: code === 0 });
    broadcast('building', { value: false });
  }

  next();
}

// ── Action endpoints ───────────────────────────────────────────────────────

app.post('/api/build', (req, res) => {
  const cfg = { ...readConfig(), ...req.body };
  const env = buildCmdEnv(cfg);
  runSequence(
    [{ cmd: 'pebble', args: ['build'], env, label: cfg.demoMode ? `demo state ${cfg.demoState}` : 'release' }],
    () => {
      lastBuiltConfig = { ...cfg };
      broadcast('last_built', lastBuiltConfig);
    }
  );
  res.json({ ok: true });
});

app.post('/api/install', (req, res) => {
  const cfg = { ...readConfig(), ...req.body };
  const env = darwinCompilerEnv();
  runSequence(
    [{ cmd: 'pebble', args: ['install', '--emulator', cfg.platform], env, label: `${cfg.platform} emulator` }],
    () => {
      lastInstalledConfig = { ...cfg };
      broadcast('last_installed', lastInstalledConfig);
    }
  );
  res.json({ ok: true });
});

app.post('/api/build-install', (req, res) => {
  const cfg = { ...readConfig(), ...req.body };
  const env = buildCmdEnv(cfg);
  runSequence(
    [
      { cmd: 'pebble', args: ['build'],                              env, label: cfg.demoMode ? `demo state ${cfg.demoState}` : 'release' },
      { cmd: 'pebble', args: ['install', '--emulator', cfg.platform], env: darwinCompilerEnv(), label: `${cfg.platform} emulator` },
    ],
    () => {
      lastBuiltConfig     = { ...cfg };
      lastInstalledConfig = { ...cfg };
      broadcast('last_built',     lastBuiltConfig);
      broadcast('last_installed', lastInstalledConfig);
    }
  );
  res.json({ ok: true });
});

app.post('/api/kill', (_req, res) => {
  if (currentProc) {
    currentProc.kill('SIGTERM');
    currentProc = null;
    isBuilding  = false;
    broadcast('info',     { text: '\nProcess killed.\n' });
    broadcast('done',     { code: -1, success: false });
    broadcast('building', { value: false });
  }
  res.json({ ok: true });
});

// ── Start ──────────────────────────────────────────────────────────────────

app.listen(PORT, '127.0.0.1', () => {
  console.log(`\n  Steady Dev Hub  →  http://localhost:${PORT}\n`);
});
