/**
 * ==============================================================
 * DISASTER MANAGEMENT SYSTEM – C++ BACKEND (DSA FOCUSED)
 * ==============================================================
 * Data Structures & Algorithms Demonstrated:
 *  - Graph (Adjacency List)
 *  - Dijkstra's Algorithm (Min-Heap / Priority Queue)
 *  - BFS / DFS for traversal
 *  - Priority Queue (Max-Heap) for high-severity emergencies
 *  - Queue (FIFO) for resource allocation
 *  - Vector for victim records
 *  - Binary Search Tree (BST) for disaster records by ID
 *  - Greedy Algorithm for resource distribution
 *
 * Integration Model:
 *  - Designed as a simple CGI-style backend.
 *  - Expects JSON on stdin with an "action" field.
 *  - Writes HTTP header + JSON response to stdout.
 *
 * NOTE: This is an academic, in-memory demo (no persistence).
 */

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <set>
#include <cstdlib>
#include <cctype>
#include <algorithm>
#include <fstream>
#include <sstream>

using namespace std;

// --------------------------------------------------------------
// Minimal JSON helpers (for controlled academic input only)
// --------------------------------------------------------------

// Read body: first line = length, then that many bytes
static string readRequestBody() {
    string line;
    if (!getline(cin, line)) return "";
    int len = atoi(line.c_str());
    if (len <= 0) return "";
    string body(len, '\0');
    cin.read(&body[0], len);
    return body;
}

// naive extraction of "key":"value" or "key":number from a flat JSON
static string jsonGetString(const string& json, const string& key) {
    string pattern = "\"" + key + "\"";
    size_t pos = json.find(pattern);
    if (pos == string::npos) return "";
    pos = json.find(':', pos);
    if (pos == string::npos) return "";
    pos++;
    while (pos < json.size() && isspace(static_cast<unsigned char>(json[pos]))) pos++;
    if (pos >= json.size()) return "";
    if (json[pos] == '\"') {
        pos++;
        string value;
        while (pos < json.size() && json[pos] != '\"') {
            value.push_back(json[pos++]);
        }
        return value;
    } else {
        string value;
        while (pos < json.size() && json[pos] != ',' && json[pos] != '}') {
            if (!isspace(static_cast<unsigned char>(json[pos])))
                value.push_back(json[pos]);
            pos++;
        }
        return value;
    }
}

static int jsonGetInt(const string& json, const string& key, int def = 0) {
    string s = jsonGetString(json, key);
    if (s.empty()) return def;
    return atoi(s.c_str());
}

// --------------------------------------------------------------
// Graph (Adjacency List) + Dijkstra + BFS + DFS
// --------------------------------------------------------------

struct Edge {
    int to;
    int weight; // distance / travel time
};

class Graph {
    int V;
    vector<vector<Edge>> adj;
    vector<string> names;

    void dfsUtil(int u, vector<bool>& vis, vector<int>& order) const {
        vis[u] = true;
        order.push_back(u);
        for (const auto& e : adj[u]) {
            if (!vis[e.to]) dfsUtil(e.to, vis, order);
        }
    }

public:
    Graph() : V(0) {}

    int addLocation(const string& name) {
        int id = V++;
        adj.push_back({});
        names.push_back(name.empty() ? ("Loc_" + to_string(id)) : name);
        return id;
    }

    bool addRoad(int u, int v, int w) {
        if (u < 0 || v < 0 || u >= V || v >= V || u == v || w <= 0) return false;
        adj[u].push_back({v, w});
        adj[v].push_back({u, w});
        return true;
    }

    int count() const { return V; }

    string getName(int id) const {
        if (id < 0 || id >= V) return "Invalid";
        return names[id];
    }

    // Dijkstra shortest path: O((V + E) log V)
    int shortestPath(int src, int dest, vector<int>* outPath = nullptr) const {
        if (src < 0 || src >= V || dest < 0 || dest >= V) return -1;
        const int INF = 1e9;
        vector<int> dist(V, INF), parent(V, -1);
        dist[src] = 0;
        using P = pair<int,int>;
        priority_queue<P, vector<P>, greater<P>> pq;
        pq.push({0, src});

        while (!pq.empty()) {
            P top = pq.top();
            pq.pop();
            int d = top.first;
            int u = top.second;
            if (d > dist[u]) continue;
            if (u == dest) break;
            for (const auto& e : adj[u]) {
                int v = e.to;
                int w = e.weight;
                if (dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    parent[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }
        if (dist[dest] == INF) return -1;
        if (outPath) {
            vector<int> path;
            for (int cur = dest; cur != -1; cur = parent[cur]) path.push_back(cur);
            reverse(path.begin(), path.end());
            *outPath = path;
        }
        return dist[dest];
    }

    // BFS traversal from src: O(V + E)
    vector<int> bfs(int src) const {
        vector<int> order;
        if (src < 0 || src >= V) return order;
        vector<bool> vis(V, false);
        queue<int> q;
        q.push(src);
        vis[src] = true;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            order.push_back(u);
            for (const auto& e : adj[u]) {
                int v = e.to;
                if (!vis[v]) {
                    vis[v] = true;
                    q.push(v);
                }
            }
        }
        return order;
    }

    // DFS traversal from src: O(V + E)
    vector<int> dfs(int src) const {
        vector<int> order;
        if (src < 0 || src >= V) return order;
        vector<bool> vis(V, false);
        dfsUtil(src, vis, order);
        return order;
    }
};

// --------------------------------------------------------------
// VictimManager – vector-based record storage
// --------------------------------------------------------------

struct Victim {
    int id;
    string name;
    int age;
    string location;
    string status;
};

class VictimManager {
    vector<Victim> victims;
    int nextId = 1;

    static string escape(const string& s) {
        string out;
        for (char c : s) {
            if (c == '\t') out += "\\t";
            else if (c == '\n') out += "\\n";
            else out += c;
        }
        return out;
    }
    static string unescape(const string& s) {
        string out;
        for (size_t i = 0; i < s.size(); i++) {
            if (i + 1 < s.size() && s[i] == '\\' && s[i+1] == 't') { out += '\t'; i++; }
            else if (i + 1 < s.size() && s[i] == '\\' && s[i+1] == 'n') { out += '\n'; i++; }
            else out += s[i];
        }
        return out;
    }

public:
    int addVictim(const string& name, int age, const string& location, const string& status) {
        Victim v{nextId++, name, age, location, status};
        victims.push_back(v);
        return v.id;
    }

    const vector<Victim>& all() const { return victims; }

    void clear() { victims.clear(); nextId = 1; }

    // Load victims from file: id\tname\tage\tlocation\tstatus per line
    void loadFromFile(const string& path) {
        clear();
        ifstream in(path);
        if (!in) return;
        string line;
        int maxId = 0;
        while (getline(in, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string idStr, name, ageStr, location, status;
            if (!getline(ss, idStr, '\t')) continue;
            if (!getline(ss, name, '\t')) continue;
            if (!getline(ss, ageStr, '\t')) continue;
            if (!getline(ss, location, '\t')) continue;
            if (!getline(ss, status, '\t')) status = "injured";
            Victim v;
            v.id = stoi(idStr);
            v.name = unescape(name);
            v.age = stoi(ageStr);
            v.location = unescape(location);
            v.status = unescape(status);
            victims.push_back(v);
            if (v.id > maxId) maxId = v.id;
        }
        nextId = maxId + 1;
    }

    void saveToFile(const string& path) const {
        ofstream out(path);
        if (!out) return;
        for (const auto& v : victims) {
            out << v.id << '\t' << escape(v.name) << '\t' << v.age << '\t'
                << escape(v.location) << '\t' << escape(v.status) << '\n';
        }
    }
};

// --------------------------------------------------------------
// DisasterManager – BST over disaster id
// --------------------------------------------------------------

struct Disaster {
    int id;
    string type;
    string location;
    int severity;
    string status; // reported / in-progress / resolved
};

struct DisasterNode {
    Disaster data;
    DisasterNode* left;
    DisasterNode* right;
    explicit DisasterNode(const Disaster& d) : data(d), left(nullptr), right(nullptr) {}
};

class DisasterManager {
    DisasterNode* root = nullptr;
    int nextId = 1;

    DisasterNode* insertNode(DisasterNode* node, const Disaster& d) {
        if (!node) return new DisasterNode(d);
        if (d.id < node->data.id) node->left = insertNode(node->left, d);
        else if (d.id > node->data.id) node->right = insertNode(node->right, d);
        return node;
    }

    DisasterNode* searchNode(DisasterNode* node, int id) const {
        if (!node) return nullptr;
        if (id == node->data.id) return node;
        if (id < node->data.id) return searchNode(node->left, id);
        return searchNode(node->right, id);
    }

    void inorder(DisasterNode* node, vector<Disaster>& out) const {
        if (!node) return;
        inorder(node->left, out);
        out.push_back(node->data);
        inorder(node->right, out);
    }

    void freeTree(DisasterNode* node) {
        if (!node) return;
        freeTree(node->left);
        freeTree(node->right);
        delete node;
    }

public:
    ~DisasterManager() { freeTree(root); }

    void clear() {
        freeTree(root);
        root = nullptr;
        nextId = 1;
    }

    // Load disasters from a simple text file (one per line):
    // id|type|location|severity|status
    void loadFromFile(const string& path) {
        clear();
        ifstream in(path);
        if (!in) return;
        string line;
        int maxId = 0;
        while (getline(in, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string idStr, type, location, sevStr, status;
            if (!getline(ss, idStr, '|')) continue;
            if (!getline(ss, type, '|')) continue;
            if (!getline(ss, location, '|')) continue;
            if (!getline(ss, sevStr, '|')) continue;
            if (!getline(ss, status, '|')) status = "reported";
            Disaster d;
            d.id = stoi(idStr);
            d.type = type;
            d.location = location;
            d.severity = stoi(sevStr);
            d.status = status;
            root = insertNode(root, d);
            if (d.id > maxId) maxId = d.id;
        }
        nextId = maxId + 1;
    }

    void saveToFile(const string& path) const {
        ofstream out(path);
        if (!out) return;
        vector<Disaster> all = allSorted();
        for (const auto& d : all) {
            out << d.id << '|' << d.type << '|' << d.location << '|' << d.severity << '|' << d.status << '\n';
        }
    }

    int addDisaster(const string& type, const string& location, int severity) {
        Disaster d;
        d.id = nextId++;
        d.type = type;
        d.location = location;
        d.severity = severity;
        d.status = "reported";
        root = insertNode(root, d);
        return d.id;
    }

    Disaster* findById(int id) {
        DisasterNode* node = searchNode(root, id);
        if (!node) return nullptr;
        return &node->data;
    }

    vector<Disaster> allSorted() const {
        vector<Disaster> out;
        inorder(root, out);
        return out;
    }
};

// --------------------------------------------------------------
// ResourceManager – Queue + Greedy allocation
// --------------------------------------------------------------

struct Resource {
    int id;
    string type;  // ambulance / food / medical
    bool available;
};

class ResourceManager {
    vector<Resource> resources;
    queue<int> availableIds; // FIFO for allocation
    int nextId = 1;

public:
    int addResource(const string& type) {
        Resource r{nextId++, type, true};
        resources.push_back(r);
        availableIds.push(r.id);
        return r.id;
    }

    const vector<Resource>& all() const { return resources; }

    void clear() { resources.clear(); availableIds = queue<int>(); nextId = 1; }

    // Load resources from file: id|type|available per line
    void loadFromFile(const string& path) {
        clear();
        ifstream in(path);
        if (!in) return;
        string line;
        int maxId = 0;
        while (getline(in, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string idStr, type, availStr;
            if (!getline(ss, idStr, '|')) continue;
            if (!getline(ss, type, '|')) continue;
            if (!getline(ss, availStr, '|')) availStr = "1";
            Resource r;
            r.id = stoi(idStr);
            r.type = type;
            r.available = (availStr == "1" || availStr == "true");
            resources.push_back(r);
            if (r.available) availableIds.push(r.id);
            if (r.id > maxId) maxId = r.id;
        }
        nextId = maxId + 1;
    }

    void saveToFile(const string& path) const {
        ofstream out(path);
        if (!out) return;
        for (const auto& r : resources) {
            out << r.id << '|' << r.type << '|' << (r.available ? "1" : "0") << '\n';
        }
    }

    // Seed default emergency resources if empty (first run)
    void seedDefaultsIfEmpty() {
        if (!resources.empty()) return;
        const char* types[] = {"first_aid", "food", "water", "clothes", "medical"};
        for (int i = 0; i < 5; i++) {
            Resource r{nextId++, types[i], true};
            resources.push_back(r);
            availableIds.push(r.id);
        }
    }

    // Greedy: assign up to numNeeded resources (FIFO)
    int allocateGreedy(int numNeeded) {
        int assigned = 0;
        while (numNeeded > 0 && !availableIds.empty()) {
            int id = availableIds.front();
            availableIds.pop();
            for (auto& r : resources) {
                if (r.id == id && r.available) {
                    r.available = false;
                    assigned++;
                    numNeeded--;
                    break;
                }
            }
        }
        return assigned;
    }

    void resetAvailability() {
        while (!availableIds.empty()) availableIds.pop();
        for (auto& r : resources) {
            if (!r.available) r.available = true;
            availableIds.push(r.id);
        }
    }
};

// --------------------------------------------------------------
// EmergencyManager – Max-Heap priority queue by severity
// --------------------------------------------------------------

struct PQEntry {
    int disasterId;
    int severity;
};

struct CompareSeverity {
    bool operator()(const PQEntry& a, const PQEntry& b) const {
        if (a.severity != b.severity) return a.severity < b.severity; // max-heap
        return a.disasterId > b.disasterId;
    }
};

class EmergencyManager {
    DisasterManager& dm;
    ResourceManager& rm;
    priority_queue<PQEntry, vector<PQEntry>, CompareSeverity> pq;

public:
    EmergencyManager(DisasterManager& d, ResourceManager& r) : dm(d), rm(r) {}

    void enqueue(int disasterId, int severity) {
        pq.push({disasterId, severity});
    }

    // Rebuild max-heap from all disasters (used after loading from file)
    void rebuildFromDisasters() {
        while (!pq.empty()) pq.pop();
        vector<Disaster> all = dm.allSorted();
        for (const auto& d : all) {
            pq.push({d.id, d.severity});
        }
    }

    string priorityListJson() {
        auto copy = pq;
        string json = "[";
        bool first = true;
        while (!copy.empty()) {
            auto e = copy.top(); copy.pop();
            Disaster* d = dm.findById(e.disasterId);
            if (!d) continue;
            if (!first) json += ",";
            first = false;
            json += "{";
            json += "\"id\":" + to_string(d->id) + ",";
            json += "\"type\":\"" + d->type + "\",";
            json += "\"location\":\"" + d->location + "\",";
            json += "\"severity\":" + to_string(d->severity) + ",";
            json += "\"status\":\"" + d->status + "\"";
            json += "}";
        }
        json += "]";
        return json;
    }

    // Greedy resource distribution:
    // Process disasters in order of severity, allocate 1 resource each if available.
    string allocateResourcesGreedy() {
        rm.resetAvailability();
        auto copy = pq;
        string json = "[";
        bool first = true;
        while (!copy.empty()) {
            auto e = copy.top(); copy.pop();
            Disaster* d = dm.findById(e.disasterId);
            if (!d) continue;
            int assigned = rm.allocateGreedy(1);
            if (!first) json += ",";
            first = false;
            json += "{";
            json += "\"disasterId\":" + to_string(d->id) + ",";
            json += "\"type\":\"" + d->type + "\",";
            json += "\"resourcesAssigned\":" + to_string(assigned);
            json += "}";
        }
        json += "]";
        return json;
    }
};

// --------------------------------------------------------------
// Global in-memory managers (per CGI process)
// --------------------------------------------------------------

static Graph g_graph;
static VictimManager g_victims;
static DisasterManager g_disasters;
static ResourceManager g_resources;
static EmergencyManager g_emergencies(g_disasters, g_resources);

// No file I/O - state stays in memory (long-running process)
static void initState() {
    g_resources.seedDefaultsIfEmpty();
    g_emergencies.rebuildFromDisasters();
}

static void saveDisasters() {
    g_emergencies.rebuildFromDisasters();
}

static void saveVictims() { }
static void saveResources() { }

// Seed demo graph (distances in meters) – shortest-path / BFS / DFS
static void ensureDemoGraph() {
    if (g_graph.count() > 0) return;
    int base = g_graph.addLocation("BaseCamp");   // 0
    int a    = g_graph.addLocation("ZoneA");      // 1
    int b    = g_graph.addLocation("ZoneB");      // 2
    int hosp = g_graph.addLocation("Hospital");   // 3
    (void)base; (void)a; (void)b; (void)hosp;
    g_graph.addRoad(0, 1, 180);   // BaseCamp -> ZoneA: 180 m
    g_graph.addRoad(1, 2, 220);   // ZoneA -> ZoneB: 220 m
    g_graph.addRoad(0, 2, 500);   // BaseCamp -> ZoneB: 500 m (direct)
    g_graph.addRoad(2, 3, 120);   // ZoneB -> Hospital: 120 m
}

// --------------------------------------------------------------
// Handlers (simple REST-style "action" router)
// --------------------------------------------------------------

static string handleAddDisaster(const string& body) {
    string type = jsonGetString(body, "type");
    string loc  = jsonGetString(body, "location");
    int severity = jsonGetInt(body, "severity", 1);
    int id = g_disasters.addDisaster(type, loc, severity);
    g_emergencies.enqueue(id, severity);
    saveDisasters();
    return string("{\"success\":true,\"id\":") + to_string(id) + "}";
}

static string handleAddVictim(const string& body) {
    string name = jsonGetString(body, "name");
    int age = jsonGetInt(body, "age", 0);
    string loc = jsonGetString(body, "location");
    string status = jsonGetString(body, "status");
    int id = g_victims.addVictim(name, age, loc, status);
    saveVictims();
    return string("{\"success\":true,\"id\":") + to_string(id) + "}";
}

static string handleAddResource(const string& body) {
    string type = jsonGetString(body, "type");
    int id = g_resources.addResource(type);
    saveResources();
    return string("{\"success\":true,\"id\":") + to_string(id) + "}";
}

static string handleClearDisasters() {
    g_disasters.clear();
    g_emergencies.rebuildFromDisasters();
    saveDisasters();
    return "{\"success\":true,\"id\":0,\"message\":\"All disasters cleared.\"}";
}

static string handleClearVictims() {
    g_victims.clear();
    saveVictims();
    return "{\"success\":true,\"id\":0,\"message\":\"All victims cleared.\"}";
}

static string handleListVictims() {
    const auto& all = g_victims.all();
    string json = "[";
    bool first = true;
    for (const auto& v : all) {
        if (!first) json += ",";
        first = false;
        json += "{";
        json += "\"id\":" + to_string(v.id) + ",";
        json += "\"name\":\"" + v.name + "\",";
        json += "\"age\":" + to_string(v.age) + ",";
        json += "\"location\":\"" + v.location + "\",";
        json += "\"status\":\"" + v.status + "\"";
        json += "}";
    }
    json += "]";
    return json;
}

static string handleListResources() {
    const auto& all = g_resources.all();
    string json = "[";
    bool first = true;
    for (const auto& r : all) {
        if (!first) json += ",";
        first = false;
        json += "{";
        json += "\"id\":" + to_string(r.id) + ",";
        json += "\"type\":\"" + r.type + "\",";
        json += "\"available\":" + string(r.available ? "true" : "false");
        json += "}";
    }
    json += "]";
    return json;
}

static string handlePriorityList() {
    return g_emergencies.priorityListJson();
}

static string handleAllocateResources() {
    return g_emergencies.allocateResourcesGreedy();
}

static string handleShortestPath(const string& body) {
    ensureDemoGraph();
    int src  = jsonGetInt(body, "src", 0);
    int dest = jsonGetInt(body, "dest", 3);
    vector<int> path;
    int dist = g_graph.shortestPath(src, dest, &path);
    if (dist < 0) {
        return "{\"success\":false,\"message\":\"No path\"}";
    }
    string json = "{";
    json += "\"success\":true,";
    json += "\"distance\":" + to_string(dist) + ",";
    json += "\"path\":[";
    bool first = true;
    for (int id : path) {
        if (!first) json += ",";
        first = false;
        json += "\"" + g_graph.getName(id) + "\"";
    }
    json += "]}";
    return json;
}

static string handleBfsDemo() {
    ensureDemoGraph();
    auto order = g_graph.bfs(0);
    string json = "[";
    bool first = true;
    for (int id : order) {
        if (!first) json += ",";
        first = false;
        json += "\"" + g_graph.getName(id) + "\"";
    }
    json += "]";
    return json;
}

static string handleDfsDemo() {
    ensureDemoGraph();
    auto order = g_graph.dfs(0);
    string json = "[";
    bool first = true;
    for (int id : order) {
        if (!first) json += ",";
        first = false;
        json += "\"" + g_graph.getName(id) + "\"";
    }
    json += "]";
    return json;
}

// --------------------------------------------------------------
// main – long-running loop (one process, in-memory state)
// Protocol: read "length\n" + body, process, write "length\n" + result
// --------------------------------------------------------------

static string processRequest(const string& body) {
    string action = jsonGetString(body, "action");
    if (action.empty()) action = "none";

    if (action == "add_disaster") return handleAddDisaster(body);
    if (action == "add_victim") return handleAddVictim(body);
    if (action == "add_resource") return handleAddResource(body);
    if (action == "clear_disasters") return handleClearDisasters();
    if (action == "clear_victims") return handleClearVictims();
    if (action == "list_victims") return handleListVictims();
    if (action == "list_resources") return handleListResources();
    if (action == "list_priority") return handlePriorityList();
    if (action == "allocate_resources") return handleAllocateResources();
    if (action == "shortest_path") return handleShortestPath(body);
    if (action == "bfs_demo") return handleBfsDemo();
    if (action == "dfs_demo") return handleDfsDemo();
    return "{\"success\":false,\"message\":\"Unknown action\"}";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    initState();

    while (cin.good()) {
        string body = readRequestBody();
        if (body.empty()) break;

        string result = processRequest(body);
        cout << result.size() << '\n' << result << flush;
    }
    return 0;
}

