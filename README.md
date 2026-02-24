## Disaster Management System – Full-Stack DSA Project

**Frontend:** HTML, CSS, basic JavaScript  
**Backend:** C++ (DSA-focused, STL only, CGI-style)

### Folder structure

```text
dsa project/
  backend/
    main.cpp        # C++ CGI backend with all DSA logic
  frontend/
    index.html      # Web UI
    style.css       # Styling
    app.js          # Fetch calls to backend
```

### DSA concepts implemented

- **Graph (Adjacency List)**: city map, nodes = locations, edges = roads (weighted)
- **Dijkstra (Min-Heap / Priority Queue)**: shortest rescue path
- **BFS / DFS**: area traversal demos from BaseCamp
- **Priority Queue (Max-Heap)**: severity-based disaster priority list
- **Queue (FIFO)**: resource allocation in `ResourceManager`
- **Vector / List**: victim records in `VictimManager`
- **Binary Search Tree (BST)**: disaster records indexed by ID in `DisasterManager`
- **Greedy Algorithm**: allocate limited resources to highest-severity disasters first

### How to build backend (Windows + g++)

In `c:\Users\idea\Desktop\dsa project\backend`:

```bash
g++ -std=c++11 -O2 -o disaster_backend.cgi main.cpp
```

Place `disaster_backend.cgi` in your web server’s `cgi-bin` (or configure a CGI path),
and ensure it is executable.

### How to open frontend

Open `frontend/index.html` in a browser.  
Update `BACKEND_URL` in `frontend/app.js` to match your CGI endpoint, e.g.:

```js
const BACKEND_URL = "/cgi-bin/disaster_backend.cgi";
```

### API actions (JSON)

Send `POST` with `Content-Type: application/json` and a body like:

- **Add disaster (BST + PQ):**
  ```json
  { "action": "add_disaster", "type": "flood", "location": "ZoneA", "severity": 8 }
  ```
- **Add victim (vector):**
  ```json
  { "action": "add_victim", "name": "Rahul", "age": 30, "location": "ZoneB", "status": "injured" }
  ```
- **Add resource (queue):**
  ```json
  { "action": "add_resource", "type": "ambulance" }
  ```
- **Disaster priority list (max-heap):**
  ```json
  { "action": "list_priority" }
  ```
- **Greedy resource allocation:**
  ```json
  { "action": "allocate_resources" }
  ```
- **Shortest path (Dijkstra):**
  ```json
  { "action": "shortest_path", "src": 0, "dest": 3 }
  ```
- **BFS / DFS traversal from BaseCamp (0):**
  ```json
  { "action": "bfs_demo" }
  { "action": "dfs_demo" }
  ```

### Sample test flow

1. Add 2–3 disaster reports with different severities.  
2. Add several resources (ambulances, food, medical kits).  
3. Call `list_priority` to see max-heap ordering.  
4. Call `allocate_resources` to see greedy distribution of limited resources.  
5. Call `shortest_path` to get the rescue route from BaseCamp to Hospital with distance and path.  
6. Use BFS / DFS demos to show traversal order on the city graph.

This structure and codebase is designed to be academically strong, clearly exposing the DSA implementations in C++ while integrating with a simple HTML/JS frontend via JSON over CGI.

# Disaster Rescue Route Optimization System

A C++ academic project demonstrating **Data Structures and Algorithms**: Graph (Adjacency List), Min-Heap, Priority Queue, Dijkstra's Algorithm, BFS, and DFS.

## Quick Start

### Compile (C++11 or later)
```bash
g++ -std=c++11 -o disaster_rescue_system disaster_rescue_system.cpp
```

### Run
```bash
./disaster_rescue_system
```
Windows:
```bash
disaster_rescue_system.exe
```

## Menu Options

| Option | Description |
|--------|-------------|
| 1 | Add location |
| 2 | Add road (with travel time in minutes) |
| 3 | Block road |
| 4 | Mark location as safe shelter |
| 5 | Request emergency rescue (priority queue by severity) |
| 6 | Find shortest path (Dijkstra) |
| 7 | Find nearest safe shelter (BFS) |
| 8 | Show reachable locations (BFS) |
| 9 | Detect isolated/disconnected areas (DFS) |
| 10 | Display city map |
| 11 | Display pending emergency requests |
| 12 | Process next emergency request |
| 0 | Exit |

## Files

- **disaster_rescue_system.cpp** – Complete source code (single file, STL only).
- **PROJECT_REPORT.md** – Full report: data structures, algorithms, pseudocode, time complexity, sample I/O, edge cases, viva Q&A.
- **README.md** – This file.

## DSA Concepts Demonstrated

- **Graph** – Weighted undirected graph using adjacency list.
- **Min-Heap** – `std::priority_queue` with `greater<>` for Dijkstra.
- **Priority Queue** – For emergency requests (max-heap by severity).
- **Dijkstra's Algorithm** – Shortest path in O((V+E) log V).
- **BFS** – Nearest shelter, reachable locations.
- **DFS** – Connected components, isolated areas.

Suitable for university DSA course submission.
