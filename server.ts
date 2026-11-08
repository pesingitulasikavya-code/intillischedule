import express, { Request, Response } from 'express';
import { createServer as createViteServer } from 'vite';
import path from 'path';
import fs from 'fs';
import { fileURLToPath } from 'url';
import { execFile, execSync } from 'child_process';
import dotenv from 'dotenv';

dotenv.config();

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const isWindows = process.platform === 'win32';
const CLI_BINARY_NAME = isWindows ? 'intellischedule_cli.exe' : 'intellischedule_cli';
const TEST_BINARY_NAME = isWindows ? 'intellischedule_tests.exe' : 'intellischedule_tests';

const CLI_BINARY_PATH = path.resolve(__dirname, 'intellischedule_cpp', 'build', CLI_BINARY_NAME);
const TEST_BINARY_PATH = path.resolve(__dirname, 'intellischedule_cpp', 'build', TEST_BINARY_NAME);
const CPP_DIR = path.resolve(__dirname, 'intellischedule_cpp');

// Ensure native IntelliSchedule C++ binary is compiled and executable
function ensureIntelliScheduleBinary(): boolean {
  try {
    if (!fs.existsSync(CLI_BINARY_PATH) || !fs.existsSync(TEST_BINARY_PATH)) {
      console.log(`[IntelliSchedule C++] Compiling native C++20 binaries via CMake...`);
      const buildDir = path.resolve(CPP_DIR, 'build');
      if (!fs.existsSync(buildDir)) fs.mkdirSync(buildDir, { recursive: true });
      execSync(`cd "${buildDir}" && cmake .. && make -j4`, { stdio: 'inherit' });
    }

    if (!isWindows) {
      if (fs.existsSync(CLI_BINARY_PATH)) {
        try { fs.chmodSync(CLI_BINARY_PATH, 0o755); } catch {}
      }
      if (fs.existsSync(TEST_BINARY_PATH)) {
        try { fs.chmodSync(TEST_BINARY_PATH, 0o755); } catch {}
      }
    }
    return fs.existsSync(CLI_BINARY_PATH);
  } catch (err: any) {
    console.warn('[IntelliSchedule C++] Compilation check note:', err?.message || err);
    return fs.existsSync(CLI_BINARY_PATH);
  }
}

// Initial check on server boot
ensureIntelliScheduleBinary();

// Helper to run intellischedule_cli
function runIntelliScheduleCli(args: string[]): Promise<any> {
  return new Promise((resolve) => {
    ensureIntelliScheduleBinary();
    execFile(CLI_BINARY_PATH, args, { cwd: CPP_DIR, timeout: 8000 }, (err, stdout, stderr) => {
      if (err) {
        console.error('[CLI Exec Error]', err.message, stderr);
        resolve(null);
        return;
      }
      if (!stdout || !stdout.trim()) {
        resolve(null);
        return;
      }
      try {
        const json = JSON.parse(stdout.trim());
        resolve(json);
      } catch {
        resolve({ raw: stdout.trim() });
      }
    });
  });
}

async function startServer() {
  const app = express();
  const PORT = Number(process.env.PORT) || 3000;

  app.use(express.json());

  // -------------------------------------------------------------
  // INTELLISCHEDULE C++20 REST API ENDPOINTS
  // -------------------------------------------------------------

  // 1. GET /api/health
  app.get('/api/health', async (_req: Request, res: Response) => {
    const cliRes = await runIntelliScheduleCli(['health']);
    res.json(cliRes || {
      status: 'ok',
      engine: 'IntelliSchedule C++20 Core Optimization Platform',
      version: '1.0.0',
      timestamp: new Date().toISOString(),
    });
  });

  // 2. GET /api/tasks
  app.get('/api/tasks', async (_req: Request, res: Response) => {
    const cliRes = await runIntelliScheduleCli(['tasks', 'list']);
    res.json(cliRes || []);
  });

  // 3. POST /api/tasks
  app.post('/api/tasks', async (req: Request, res: Response) => {
    const task = req.body;
    if (!task.title) {
      res.status(400).json({ error: 'Task title is required' });
      return;
    }
    if (task.duration !== undefined && task.duration <= 0) {
      res.status(400).json({ error: 'Task duration must be strictly positive (> 0)' });
      return;
    }
    if (!task.id) {
      task.id = 'TASK-' + Date.now();
    }
    const cliRes = await runIntelliScheduleCli(['tasks', 'save', JSON.stringify(task)]);
    if (cliRes && cliRes.task) {
      res.status(201).json(cliRes);
    } else {
      res.status(400).json(cliRes || { error: 'Failed to create task' });
    }
  });

  // 4. PUT /api/tasks/:id
  app.put('/api/tasks/:id', async (req: Request, res: Response) => {
    const task = req.body;
    task.id = req.params.id;
    if (task.duration !== undefined && task.duration <= 0) {
      res.status(400).json({ error: 'Task duration must be strictly positive (> 0)' });
      return;
    }
    const cliRes = await runIntelliScheduleCli(['tasks', 'save', JSON.stringify(task)]);
    if (cliRes && cliRes.task) {
      res.json(cliRes);
    } else {
      res.status(400).json(cliRes || { error: 'Failed to update task' });
    }
  });

  // 5. DELETE /api/tasks/:id
  app.delete('/api/tasks/:id', async (req: Request, res: Response) => {
    const cliRes = await runIntelliScheduleCli(['tasks', 'delete', req.params.id]);
    res.json(cliRes || { success: true, id: req.params.id });
  });

  // 6. POST /api/schedule
  app.post('/api/schedule', async (req: Request, res: Response) => {
    const payload = JSON.stringify(req.body);
    const cliRes = await runIntelliScheduleCli(['schedule', payload]);
    if (cliRes) {
      res.json(cliRes);
    } else {
      res.status(500).json({ success: false, error: 'C++ scheduling engine execution failed' });
    }
  });

  // 7. POST /api/compare
  app.post('/api/compare', async (req: Request, res: Response) => {
    const payload = JSON.stringify(req.body);
    const cliRes = await runIntelliScheduleCli(['compare', payload]);
    if (cliRes) {
      res.json(cliRes);
    } else {
      res.status(500).json({ success: false, error: 'C++ comparison engine execution failed' });
    }
  });

  // 8. GET /api/scenarios/:id
  app.get('/api/scenarios/:id', async (req: Request, res: Response) => {
    const cliRes = await runIntelliScheduleCli(['scenarios', 'get', req.params.id]);
    if (cliRes && !cliRes.error) {
      res.json(cliRes);
    } else {
      res.status(404).json({ error: 'Scenario not found' });
    }
  });

  // 9. GET /api/scenarios
  app.get('/api/scenarios', async (_req: Request, res: Response) => {
    const cliRes = await runIntelliScheduleCli(['scenarios', 'list']);
    res.json(cliRes || []);
  });

  // 10. POST /api/tests/run - Run the automated C++ test suite and return output
  app.post('/api/tests/run', async (_req: Request, res: Response) => {
    execFile(TEST_BINARY_PATH, [], { cwd: CPP_DIR, timeout: 10000 }, (err, stdout, stderr) => {
      const output = (stdout || '') + (stderr || '');
      const success = !err && output.includes('ALL ASSERTS VERIFIED');
      res.json({
        success,
        exitCode: err ? (err as any).code : 0,
        output,
        timestamp: new Date().toISOString(),
      });
    });
  });

  // Vite Middleware / Static Files
  if (process.env.NODE_ENV === 'production') {
    app.use(express.static(path.resolve(__dirname, 'dist')));
    app.get('*', (_req: Request, res: Response) => {
      res.sendFile(path.resolve(__dirname, 'dist', 'index.html'));
    });
  } else {
    const vite = await createViteServer({
      server: { middlewareMode: true },
      appType: 'spa',
    });
    app.use(vite.middlewares);
  }

  app.listen(PORT, '0.0.0.0', () => {
    console.log(`IntelliSchedule Server running on http://localhost:${PORT}`);
  });
}

startServer().catch((err) => {
  console.error('Failed to start server:', err);
});
