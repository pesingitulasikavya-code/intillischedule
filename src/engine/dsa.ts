// Advanced DSA Engine: Trees, Graphs, Sorting, Searching, Greedy, Dynamic Programming, and Backtracking

// ==========================================
// 1. LINEAR STRUCTURES
// ==========================================

export class Stack<T = number> {
  items: T[];

  constructor(items: T[] = []) {
    this.items = [...items];
  }

  push(v: T): T[] {
    this.items.push(v);
    return this.items;
  }

  pop(): T | undefined {
    return this.items.pop();
  }

  peek(): T | undefined {
    return this.items[this.items.length - 1];
  }

  clear(): void {
    this.items = [];
  }
}

export class Queue<T = number> {
  items: T[];

  constructor(items: T[] = []) {
    this.items = [...items];
  }

  enqueue(v: T): T[] {
    this.items.push(v);
    return this.items;
  }

  dequeue(): T | undefined {
    return this.items.shift();
  }

  front(): T | undefined {
    return this.items[0];
  }

  clear(): void {
    this.items = [];
  }
}

export interface ListNode<T> {
  value: T;
  next: ListNode<T> | null;
}

export class LinkedList<T = number> {
  head: ListNode<T> | null = null;

  constructor(values: T[] = []) {
    values.forEach((v) => this.append(v));
  }

  append(value: T): void {
    const node: ListNode<T> = { value, next: null };
    if (!this.head) {
      this.head = node;
      return;
    }
    let n = this.head;
    while (n.next) n = n.next;
    n.next = node;
  }

  prepend(value: T): void {
    this.head = { value, next: this.head };
  }

  insertAt(value: T, index: number): boolean {
    if (index < 0) return false;
    if (index === 0) {
      this.prepend(value);
      return true;
    }
    const node: ListNode<T> = { value, next: null };
    let cur = this.head;
    let i = 0;
    while (cur && i < index - 1) {
      cur = cur.next;
      i++;
    }
    if (!cur) return false;
    node.next = cur.next;
    cur.next = node;
    return true;
  }

  remove(value: T): boolean {
    if (!this.head) return false;
    if (this.head.value === value) {
      this.head = this.head.next;
      return true;
    }
    let n = this.head;
    while (n.next && n.next.value !== value) {
      n = n.next;
    }
    if (!n.next) return false;
    n.next = n.next.next;
    return true;
  }

  reverse(): void {
    let prev: ListNode<T> | null = null;
    let cur = this.head;
    while (cur) {
      const next = cur.next;
      cur.next = prev;
      prev = cur;
      cur = next;
    }
    this.head = prev;
  }

  toArray(): T[] {
    const out: T[] = [];
    let n = this.head;
    while (n) {
      out.push(n.value);
      n = n.next;
    }
    return out;
  }
}

// ==========================================
// 2. TREE STRUCTURES (BST, AVL, HEAPS)
// ==========================================

export interface TreeNodeData {
  id: string;
  value: number;
  left: TreeNodeData | null;
  right: TreeNodeData | null;
  height?: number;
  balanceFactor?: number;
}

export class BSTNode implements TreeNodeData {
  id: string;
  value: number;
  left: BSTNode | null = null;
  right: BSTNode | null = null;

  constructor(value: number) {
    this.id = 'node_' + Math.random().toString(36).substring(2, 8);
    this.value = value;
  }
}

export class BinarySearchTree {
  root: BSTNode | null = null;

  constructor(initialValues: number[] = []) {
    initialValues.forEach((v) => this.insert(v));
  }

  insert(value: number): void {
    const newNode = new BSTNode(value);
    if (!this.root) {
      this.root = newNode;
      return;
    }
    let cur = this.root;
    while (true) {
      if (value < cur.value) {
        if (!cur.left) {
          cur.left = newNode;
          break;
        }
        cur = cur.left;
      } else if (value > cur.value) {
        if (!cur.right) {
          cur.right = newNode;
          break;
        }
        cur = cur.right;
      } else {
        // Value already present
        break;
      }
    }
  }

  search(value: number): number[] {
    const path: number[] = [];
    let cur = this.root;
    while (cur) {
      path.push(cur.value);
      if (value === cur.value) return path;
      if (value < cur.value) cur = cur.left;
      else cur = cur.right;
    }
    return path;
  }

  delete(value: number): void {
    this.root = this._deleteNode(this.root, value);
  }

  private _deleteNode(node: BSTNode | null, value: number): BSTNode | null {
    if (!node) return null;
    if (value < node.value) {
      node.left = this._deleteNode(node.left, value);
      return node;
    } else if (value > node.value) {
      node.right = this._deleteNode(node.right, value);
      return node;
    } else {
      if (!node.left && !node.right) return null;
      if (!node.left) return node.right;
      if (!node.right) return node.left;

      let min = node.right;
      while (min.left) min = min.left;
      node.value = min.value;
      node.right = this._deleteNode(node.right, min.value);
      return node;
    }
  }

  inorder(node: BSTNode | null = this.root, res: number[] = []): number[] {
    if (!node) return res;
    this.inorder(node.left, res);
    res.push(node.value);
    this.inorder(node.right, res);
    return res;
  }

  preorder(node: BSTNode | null = this.root, res: number[] = []): number[] {
    if (!node) return res;
    res.push(node.value);
    this.preorder(node.left, res);
    this.preorder(node.right, res);
    return res;
  }

  postorder(node: BSTNode | null = this.root, res: number[] = []): number[] {
    if (!node) return res;
    this.postorder(node.left, res);
    this.postorder(node.right, res);
    res.push(node.value);
    return res;
  }

  bfs(): number[] {
    if (!this.root) return [];
    const q: BSTNode[] = [this.root];
    const res: number[] = [];
    while (q.length) {
      const n = q.shift()!;
      res.push(n.value);
      if (n.left) q.push(n.left);
      if (n.right) q.push(n.right);
    }
    return res;
  }
}

// AVL Tree Node & Engine
export class AVLNode implements TreeNodeData {
  id: string;
  value: number;
  height: number = 1;
  balanceFactor: number = 0;
  left: AVLNode | null = null;
  right: AVLNode | null = null;

  constructor(value: number) {
    this.id = 'avl_' + Math.random().toString(36).substring(2, 8);
    this.value = value;
  }
}

export class AVLTree {
  root: AVLNode | null = null;
  lastRotation: string = 'None';

  constructor(initialValues: number[] = []) {
    initialValues.forEach((v) => this.insert(v));
  }

  private getHeight(node: AVLNode | null): number {
    return node ? node.height : 0;
  }

  private getBalance(node: AVLNode | null): number {
    return node ? this.getHeight(node.left) - this.getHeight(node.right) : 0;
  }

  private updateMeta(node: AVLNode): void {
    node.height = 1 + Math.max(this.getHeight(node.left), this.getHeight(node.right));
    node.balanceFactor = this.getBalance(node);
  }

  private rotateRight(y: AVLNode): AVLNode {
    const x = y.left!;
    const T2 = x.right;
    x.right = y;
    y.left = T2;
    this.updateMeta(y);
    this.updateMeta(x);
    return x;
  }

  private rotateLeft(x: AVLNode): AVLNode {
    const y = x.right!;
    const T2 = y.left;
    y.left = x;
    x.right = T2;
    this.updateMeta(x);
    this.updateMeta(y);
    return y;
  }

  insert(value: number): void {
    this.lastRotation = 'None';
    this.root = this._insertNode(this.root, value);
  }

  private _insertNode(node: AVLNode | null, value: number): AVLNode {
    if (!node) return new AVLNode(value);

    if (value < node.value) {
      node.left = this._insertNode(node.left, value);
    } else if (value > node.value) {
      node.right = this._insertNode(node.right, value);
    } else {
      return node; // Duplicate keys not allowed
    }

    this.updateMeta(node);
    const balance = node.balanceFactor;

    // LL Rotation
    if (balance > 1 && value < node.left!.value) {
      this.lastRotation = 'LL Rotation (Right Rotate)';
      return this.rotateRight(node);
    }
    // RR Rotation
    if (balance < -1 && value > node.right!.value) {
      this.lastRotation = 'RR Rotation (Left Rotate)';
      return this.rotateLeft(node);
    }
    // LR Rotation
    if (balance > 1 && value > node.left!.value) {
      this.lastRotation = 'LR Rotation (Left-Right Rotate)';
      node.left = this.rotateLeft(node.left!);
      return this.rotateRight(node);
    }
    // RL Rotation
    if (balance < -1 && value < node.right!.value) {
      this.lastRotation = 'RL Rotation (Right-Left Rotate)';
      node.right = this.rotateRight(node.right!);
      return this.rotateLeft(node);
    }

    return node;
  }
}

// Min/Max Heap
export class BinaryHeap {
  items: number[] = [];
  isMaxHeap: boolean;

  constructor(items: number[] = [], isMaxHeap = false) {
    this.isMaxHeap = isMaxHeap;
    items.forEach((x) => this.insert(x));
  }

  private compare(a: number, b: number): boolean {
    return this.isMaxHeap ? a > b : a < b;
  }

  insert(v: number): void {
    this.items.push(v);
    this.bubbleUp(this.items.length - 1);
  }

  private bubbleUp(idx: number): void {
    while (idx > 0) {
      const parent = Math.floor((idx - 1) / 2);
      if (this.compare(this.items[idx], this.items[parent])) {
        [this.items[idx], this.items[parent]] = [this.items[parent], this.items[idx]];
        idx = parent;
      } else {
        break;
      }
    }
  }

  extractRoot(): number | undefined {
    if (!this.items.length) return undefined;
    const root = this.items[0];
    const last = this.items.pop()!;
    if (this.items.length) {
      this.items[0] = last;
      this.sinkDown(0);
    }
    return root;
  }

  private sinkDown(idx: number): void {
    const len = this.items.length;
    while (true) {
      let target = idx;
      const left = 2 * idx + 1;
      const right = 2 * idx + 2;

      if (left < len && this.compare(this.items[left], this.items[target])) {
        target = left;
      }
      if (right < len && this.compare(this.items[right], this.items[target])) {
        target = right;
      }
      if (target !== idx) {
        [this.items[idx], this.items[target]] = [this.items[target], this.items[idx]];
        idx = target;
      } else {
        break;
      }
    }
  }
}

// ==========================================
// 3. SORTING ALGORITHMS WITH STEP-BY-STEP TRACKING
// ==========================================

export interface SortStep {
  array: number[];
  active: number[];
  sorted?: number[];
  message: string;
}

export function bubbleSortSteps(input: number[]): SortStep[] {
  const a = [...input];
  const steps: SortStep[] = [{ array: [...a], active: [], message: 'Initial array state.' }];
  for (let i = 0; i < a.length - 1; i++) {
    for (let j = 0; j < a.length - i - 1; j++) {
      steps.push({ array: [...a], active: [j, j + 1], message: `Compare elements at index ${j} (${a[j]}) and ${j + 1} (${a[j + 1]})` });
      if (a[j] > a[j + 1]) {
        [a[j], a[j + 1]] = [a[j + 1], a[j]];
        steps.push({ array: [...a], active: [j, j + 1], message: `Swapped: ${a[j + 1]} was smaller than ${a[j]}` });
      }
    }
  }
  steps.push({ array: [...a], active: [], message: 'Array is completely sorted.' });
  return steps;
}

export function selectionSortSteps(input: number[]): SortStep[] {
  const a = [...input];
  const steps: SortStep[] = [{ array: [...a], active: [], message: 'Initial array state.' }];
  for (let i = 0; i < a.length - 1; i++) {
    let min = i;
    for (let j = i + 1; j < a.length; j++) {
      steps.push({ array: [...a], active: [min, j], message: `Checking if ${a[j]} at index ${j} is smaller than current minimum ${a[min]}` });
      if (a[j] < a[min]) min = j;
    }
    if (min !== i) {
      [a[i], a[min]] = [a[min], a[i]];
    }
    steps.push({ array: [...a], active: [i, min], message: `Placed smallest remaining element ${a[i]} into position ${i}` });
  }
  steps.push({ array: [...a], active: [], message: 'Array is sorted.' });
  return steps;
}

export function insertionSortSteps(input: number[]): SortStep[] {
  const a = [...input];
  const steps: SortStep[] = [{ array: [...a], active: [], message: 'Initial array state.' }];
  for (let i = 1; i < a.length; i++) {
    const key = a[i];
    let j = i - 1;
    while (j >= 0 && a[j] > key) {
      a[j + 1] = a[j];
      steps.push({ array: [...a], active: [j, j + 1], message: `Shift ${a[j]} right to make room for ${key}` });
      j--;
    }
    a[j + 1] = key;
    steps.push({ array: [...a], active: [j + 1], message: `Inserted ${key} into sorted subarray at index ${j + 1}` });
  }
  steps.push({ array: [...a], active: [], message: 'Array is sorted.' });
  return steps;
}

export function mergeSortSteps(input: number[]): SortStep[] {
  const a = [...input];
  const steps: SortStep[] = [{ array: [...a], active: [], message: 'Begin Divide and Conquer: Merge Sort.' }];

  function merge(start: number, mid: number, end: number) {
    const left = a.slice(start, mid + 1);
    const right = a.slice(mid + 1, end + 1);
    let i = 0, j = 0, k = start;

    steps.push({
      array: [...a],
      active: [start, end],
      message: `Merging subarrays: [${left.join(', ')}] and [${right.join(', ')}]`,
    });

    while (i < left.length && j < right.length) {
      if (left[i] <= right[j]) {
        a[k] = left[i++];
      } else {
        a[k] = right[j++];
      }
      steps.push({ array: [...a], active: [k], message: `Merged ${a[k]} into position ${k}` });
      k++;
    }
    while (i < left.length) {
      a[k] = left[i++];
      steps.push({ array: [...a], active: [k], message: `Appended remaining left element ${a[k]}` });
      k++;
    }
    while (j < right.length) {
      a[k] = right[j++];
      steps.push({ array: [...a], active: [k], message: `Appended remaining right element ${a[k]}` });
      k++;
    }
  }

  function divide(start: number, end: number) {
    if (start >= end) return;
    const mid = Math.floor((start + end) / 2);
    steps.push({
      array: [...a],
      active: [start, mid, end],
      message: `Divide range [${start}..${end}] at middle index ${mid}`,
    });
    divide(start, mid);
    divide(mid + 1, end);
    merge(start, mid, end);
  }

  divide(0, a.length - 1);
  steps.push({ array: [...a], active: [], message: 'Merge sort complete! Array is sorted in O(n log n).' });
  return steps;
}

export function quickSortSteps(input: number[]): SortStep[] {
  const a = [...input];
  const steps: SortStep[] = [{ array: [...a], active: [], message: 'Begin Divide and Conquer: Quick Sort.' }];

  function partition(low: number, high: number): number {
    const pivot = a[high];
    steps.push({ array: [...a], active: [high], message: `Selected pivot value ${pivot} at index ${high}` });
    let i = low - 1;

    for (let j = low; j < high; j++) {
      steps.push({ array: [...a], active: [j, high], message: `Comparing element ${a[j]} against pivot ${pivot}` });
      if (a[j] < pivot) {
        i++;
        [a[i], a[j]] = [a[j], a[i]];
        if (i !== j) {
          steps.push({ array: [...a], active: [i, j], message: `Swapped ${a[j]} and ${a[i]} to move values smaller than pivot to the left` });
        }
      }
    }
    [a[i + 1], a[high]] = [a[high], a[i + 1]];
    steps.push({ array: [...a], active: [i + 1], message: `Placed pivot ${pivot} in its final sorted position at index ${i + 1}` });
    return i + 1;
  }

  function sort(low: number, high: number) {
    if (low < high) {
      const pi = partition(low, high);
      sort(low, pi - 1);
      sort(pi + 1, high);
    }
  }

  sort(0, a.length - 1);
  steps.push({ array: [...a], active: [], message: 'Quick Sort complete. All partitions resolved.' });
  return steps;
}

export function heapSortSteps(input: number[]): SortStep[] {
  const a = [...input];
  const n = a.length;
  const steps: SortStep[] = [{ array: [...a], active: [], message: 'Begin Heap Sort: Building Max-Heap.' }];

  function heapify(len: number, i: number) {
    let largest = i;
    const left = 2 * i + 1;
    const right = 2 * i + 2;

    if (left < len && a[left] > a[largest]) largest = left;
    if (right < len && a[right] > a[largest]) largest = right;

    if (largest !== i) {
      [a[i], a[largest]] = [a[largest], a[i]];
      steps.push({ array: [...a], active: [i, largest], message: `Heapify swap: placed larger child ${a[i]} at parent index ${i}` });
      heapify(len, largest);
    }
  }

  for (let i = Math.floor(n / 2) - 1; i >= 0; i--) {
    heapify(n, i);
  }
  steps.push({ array: [...a], active: [], message: 'Max-Heap constructed. Extracting maximum elements to end of array.' });

  for (let i = n - 1; i > 0; i--) {
    [a[0], a[i]] = [a[i], a[0]];
    steps.push({ array: [...a], active: [0, i], message: `Extracted maximum ${a[i]} to sorted position ${i}` });
    heapify(i, 0);
  }

  steps.push({ array: [...a], active: [], message: 'Heap Sort complete in O(n log n).' });
  return steps;
}

// ==========================================
// 4. SEARCHING ALGORITHMS
// ==========================================

export interface SearchStep {
  index: number;
  left?: number;
  right?: number;
  found?: boolean;
  message: string;
}

export function linearSearchSteps(a: number[], target: number): SearchStep[] {
  const s: SearchStep[] = [];
  for (let i = 0; i < a.length; i++) {
    s.push({ index: i, message: `Checking index ${i}: element is ${a[i]}` });
    if (a[i] === target) {
      s.push({ index: i, found: true, message: `Success! Target ${target} located at index ${i}.` });
      return s;
    }
  }
  s.push({ index: -1, found: false, message: `Target ${target} was not found in the array.` });
  return s;
}

export function binarySearchSteps(a: number[], target: number): SearchStep[] {
  const sorted = [...a].sort((x, y) => x - y);
  const s: SearchStep[] = [];
  let l = 0;
  let r = sorted.length - 1;
  while (l <= r) {
    const m = Math.floor((l + r) / 2);
    s.push({ index: m, left: l, right: r, message: `Middle index ${m} has value ${sorted[m]} (Search interval: [${l}..${r}])` });
    if (sorted[m] === target) {
      s.push({ index: m, left: l, right: r, found: true, message: `Target ${target} matched at index ${m}!` });
      return s;
    }
    if (sorted[m] < target) {
      l = m + 1;
    } else {
      r = m - 1;
    }
  }
  s.push({ index: -1, found: false, message: `Target ${target} is not in the array.` });
  return s;
}

// ==========================================
// 5. GRAPH ALGORITHMS (BFS, DFS, DIJKSTRA, MST)
// ==========================================

export interface GraphStep {
  current: string;
  queue: string[];
  visited: string[];
  distances?: Record<string, number>;
  message: string;
}

export function bfsSteps(graph: Record<string, string[]>, start: string): GraphStep[] {
  const visited = new Set<string>([start]);
  const q: string[] = [start];
  const steps: GraphStep[] = [
    { current: start, queue: [...q], visited: [start], message: `Initialize BFS: starting at vertex "${start}".` },
  ];

  while (q.length) {
    const v = q.shift()!;
    const neighbors = graph[v] || [];
    steps.push({ current: v, queue: [...q], visited: Array.from(visited), message: `Dequeue vertex "${v}". Inspecting incident edges: ${neighbors.join(', ') || 'None'}` });
    for (const n of neighbors) {
      if (!visited.has(n)) {
        visited.add(n);
        q.push(n);
        steps.push({ current: v, queue: [...q], visited: Array.from(visited), message: `Discovered unvisited neighbor "${n}". Added to FIFO queue.` });
      }
    }
  }
  steps.push({ current: '', queue: [], visited: Array.from(visited), message: `BFS traversal completed. Visited ${visited.size} vertices.` });
  return steps;
}

export function dfsSteps(graph: Record<string, string[]>, start: string): GraphStep[] {
  const visited = new Set<string>();
  const stack: string[] = [start];
  const steps: GraphStep[] = [
    { current: start, queue: [...stack], visited: [], message: `Initialize DFS: push root "${start}" onto recursion stack.` },
  ];

  function dfs(v: string) {
    visited.add(v);
    steps.push({ current: v, queue: Array.from(visited), visited: Array.from(visited), message: `Visit vertex "${v}". Exploring branches deeply.` });
    const neighbors = graph[v] || [];
    for (const n of neighbors) {
      if (!visited.has(n)) {
        dfs(n);
        steps.push({ current: v, queue: Array.from(visited), visited: Array.from(visited), message: `Backtrack to vertex "${v}".` });
      }
    }
  }

  dfs(start);
  steps.push({ current: '', queue: [], visited: Array.from(visited), message: `DFS exploration completed. All reachable paths resolved.` });
  return steps;
}

export interface DijkstraStep {
  current: string;
  visited: string[];
  distances: Record<string, number>;
  message: string;
}

export function dijkstraSteps(graph: Record<string, { to: string; weight: number }[]>, start: string): DijkstraStep[] {
  const nodes = Object.keys(graph);
  const distances: Record<string, number> = {};
  nodes.forEach((n) => (distances[n] = Infinity));
  distances[start] = 0;

  const unvisited = new Set<string>(nodes);
  const visited: string[] = [];
  const steps: DijkstraStep[] = [
    { current: start, visited: [], distances: { ...distances }, message: `Set distance to source vertex "${start}" = 0, all others = ∞` },
  ];

  while (unvisited.size) {
    let closest: string | null = null;
    let minD = Infinity;
    unvisited.forEach((u) => {
      if (distances[u] < minD) {
        minD = distances[u];
        closest = u;
      }
    });

    if (!closest || minD === Infinity) break;

    unvisited.delete(closest);
    visited.push(closest);

    steps.push({
      current: closest,
      visited: [...visited],
      distances: { ...distances },
      message: `Selected unvisited vertex "${closest}" with minimum distance ${minD}. Relaxing adjacent edges.`,
    });

    const edges = graph[closest] || [];
    for (const edge of edges) {
      if (unvisited.has(edge.to)) {
        const alt = distances[closest] + edge.weight;
        if (alt < distances[edge.to]) {
          distances[edge.to] = alt;
          steps.push({
            current: closest,
            visited: [...visited],
            distances: { ...distances },
            message: `Relax edge (${closest} → ${edge.to}, wt ${edge.weight}): new shortest distance = ${alt}`,
          });
        }
      }
    }
  }

  steps.push({
    current: '',
    visited: [...visited],
    distances: { ...distances },
    message: `Dijkstra's shortest path complete. Final minimum distances determined for all vertices.`,
  });

  return steps;
}

// ==========================================
// 6. GREEDY & DYNAMIC PROGRAMMING STEPS
// ==========================================

export interface Job {
  id: string;
  deadline: number;
  profit: number;
}

export interface JobStep {
  currentJob: string;
  scheduled: (string | null)[];
  totalProfit: number;
  message: string;
}

export function jobSequencingSteps(jobs: Job[], maxSlots = 3): JobStep[] {
  const sorted = [...jobs].sort((a, b) => b.profit - a.profit);
  const slots: (string | null)[] = new Array(maxSlots).fill(null);
  let totalProfit = 0;
  const steps: JobStep[] = [
    { currentJob: '', scheduled: [...slots], totalProfit: 0, message: `Sort jobs in descending profit order.` },
  ];

  for (const job of sorted) {
    let placed = false;
    for (let slot = Math.min(maxSlots, job.deadline) - 1; slot >= 0; slot--) {
      if (slots[slot] === null) {
        slots[slot] = `${job.id} (+$${job.profit})`;
        totalProfit += job.profit;
        placed = true;
        steps.push({
          currentJob: job.id,
          scheduled: [...slots],
          totalProfit,
          message: `Scheduled Job ${job.id} (Profit: ${job.profit}, Deadline: ${job.deadline}) into slot ${slot + 1}.`,
        });
        break;
      }
    }
    if (!placed) {
      steps.push({
        currentJob: job.id,
        scheduled: [...slots],
        totalProfit,
        message: `Job ${job.id} discarded: no available time slots prior to deadline ${job.deadline}.`,
      });
    }
  }

  steps.push({
    currentJob: '',
    scheduled: [...slots],
    totalProfit,
    message: `Job sequencing finished. Maximum profit achieved: $${totalProfit}.`,
  });

  return steps;
}

// 8-Queens Backtracking Steps
export interface QueensStep {
  board: number[]; // board[row] = col
  activeRow: number;
  activeCol: number;
  isConflict: boolean;
  message: string;
}

export function eightQueensSteps(n = 4): QueensStep[] {
  const steps: QueensStep[] = [];
  const board: number[] = new Array(n).fill(-1);

  function isSafe(row: number, col: number): boolean {
    for (let i = 0; i < row; i++) {
      if (board[i] === col) return false;
      if (Math.abs(board[i] - col) === Math.abs(i - row)) return false;
    }
    return true;
  }

  function solve(row: number): boolean {
    if (row === n) {
      steps.push({
        board: [...board],
        activeRow: row - 1,
        activeCol: board[row - 1],
        isConflict: false,
        message: `All ${n} Queens placed successfully without conflicts! Solution found.`,
      });
      return true;
    }

    for (let col = 0; col < n; col++) {
      board[row] = col;
      const safe = isSafe(row, col);
      steps.push({
        board: [...board],
        activeRow: row,
        activeCol: col,
        isConflict: !safe,
        message: safe
          ? `Placed Queen at (Row ${row + 1}, Col ${col + 1}): Safe configuration.`
          : `Conflict at (Row ${row + 1}, Col ${col + 1}): Attack along row, column, or diagonal.`,
      });

      if (safe) {
        if (solve(row + 1)) return true;
      }

      board[row] = -1;
      steps.push({
        board: [...board],
        activeRow: row,
        activeCol: col,
        isConflict: false,
        message: `Backtrack: removed Queen from (Row ${row + 1}, Col ${col + 1}). Trying next candidate.`,
      });
    }
    return false;
  }

  solve(0);
  return steps;
}
