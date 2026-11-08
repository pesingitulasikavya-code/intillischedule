import axios from 'axios';

export interface User {
  id: string;
  name: string;
  email: string;
}

export const api = axios.create({
  baseURL: import.meta.env.VITE_API_URL || '/api',
});

api.interceptors.request.use((config) => {
  const token = localStorage.getItem('dsahub_token');
  if (token) {
    config.headers.Authorization = `Bearer ${token}`;
  }
  return config;
});

export function getCurrentUser(): User | null {
  try {
    const u = localStorage.getItem('dsahub_user');
    return u ? JSON.parse(u) : null;
  } catch {
    return null;
  }
}

export async function saveProgress(topicId: string): Promise<string[]> {
  const user = getCurrentUser();
  if (!user) {
    throw new Error('NOT_AUTHENTICATED');
  }

  const res = await api.post('/progress', { topicId });
  if (res.data?.completedTopics) {
    localStorage.setItem(`dsahub_done_${user.email}`, JSON.stringify(res.data.completedTopics));
    return res.data.completedTopics;
  }

  // Fallback local per-user
  const key = `dsahub_done_${user.email}`;
  const list: string[] = JSON.parse(localStorage.getItem(key) || '[]');
  const idx = list.indexOf(topicId);
  if (idx === -1) {
    list.push(topicId);
  } else {
    list.splice(idx, 1);
  }
  localStorage.setItem(key, JSON.stringify(list));
  return list;
}

export async function getProgress(): Promise<string[]> {
  const user = getCurrentUser();
  if (!user) {
    return []; // Guests have 0 default progress
  }

  try {
    const res = await api.get('/progress');
    if (Array.isArray(res.data)) {
      localStorage.setItem(`dsahub_done_${user.email}`, JSON.stringify(res.data));
      return res.data;
    }
  } catch {
    // Return cached per-user progress
  }

  try {
    const local = localStorage.getItem(`dsahub_done_${user.email}`);
    return local ? JSON.parse(local) : [];
  } catch {
    return [];
  }
}

export async function loginUser(email: string, password: string): Promise<{ token: string; user: User; completedTopics?: string[] }> {
  const res = await api.post('/auth/login', { email, password });
  if (res.data?.token) {
    localStorage.setItem('dsahub_token', res.data.token);
    localStorage.setItem('dsahub_user', JSON.stringify(res.data.user));
    const completed = Array.isArray(res.data.completedTopics) ? res.data.completedTopics : [];
    localStorage.setItem(`dsahub_done_${res.data.user.email}`, JSON.stringify(completed));
  }
  return res.data;
}

export async function registerUser(name: string, email: string, password: string): Promise<{ token: string; user: User; completedTopics?: string[] }> {
  const res = await api.post('/auth/register', { name, email, password });
  if (res.data?.token) {
    localStorage.setItem('dsahub_token', res.data.token);
    localStorage.setItem('dsahub_user', JSON.stringify(res.data.user));
    const completed = Array.isArray(res.data.completedTopics) ? res.data.completedTopics : [];
    localStorage.setItem(`dsahub_done_${res.data.user.email}`, JSON.stringify(completed));
  }
  return res.data;
}

export function logoutUser(): void {
  localStorage.removeItem('dsahub_token');
  localStorage.removeItem('dsahub_user');
}
