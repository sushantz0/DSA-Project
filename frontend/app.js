// Frontend JS: talks to C++ backend via Node server and shows friendly text.
//
// Backend endpoint (Node server proxying to C++ backend)
// Run:   node server.js   (in project root)
// Open:  http://localhost:8000
const BACKEND_URL = "/";

async function postAction(action, payload) {
  const output = document.getElementById("output");
  try {
    const res = await fetch(BACKEND_URL, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ action, ...payload }),
    });

    const raw = await res.text();

    // Strip any HTTP headers that backend might include (Status, Content-Type, etc.)
    let text = raw;
    const headerEnd = raw.indexOf("\r\n\r\n");
    const headerEnd2 = raw.indexOf("\n\n");
    if (headerEnd !== -1) text = raw.substring(headerEnd + 4);
    else if (headerEnd2 !== -1) text = raw.substring(headerEnd2 + 2);

    let data = null;
    if (text.trim().length > 0) {
      try {
        data = JSON.parse(text);
      } catch (_) {
        output.textContent = raw || "Invalid response from server.";
        return;
      }
    }

    if (!res.ok && data && data.error) {
      output.textContent = "Backend error: " + data.error;
      return;
    }

    output.textContent = formatOutput(action, data);
  } catch (err) {
    output.textContent = "Error: " + err.message;
  }
}

function formatOutput(action, data) {
  // Create disaster / victim / resource
  if (action === "add_disaster") {
    if (data && data.success) return `Disaster added successfully. ID = ${data.id}.`;
    return "Failed to add disaster.";
  }
  if (action === "add_victim") {
    if (data && data.success) return `Victim added successfully. ID = ${data.id}.`;
    return "Failed to add victim.";
  }
  if (action === "add_resource") {
    if (data && data.success) return `Resource added successfully. ID = ${data.id}.`;
    return "Failed to add resource.";
  }

  if (action === "clear_disasters") {
    if (data && (data.success === true || data.message)) return "All disasters cleared.";
    if (data && data.error) return "Error: " + data.error;
    return "Failed to clear disasters.";
  }

  if (action === "clear_victims") {
    if (data && (data.success === true || data.message)) return "All victims cleared.";
    if (data && data.error) return "Error: " + data.error;
    return "Failed to clear victims.";
  }

  // Victim records – show names and info
  if (action === "list_victims") {
    if (!Array.isArray(data) || data.length === 0) {
      return "No victim records found.";
    }
    return (
      "Victim Records:\n" +
      data
        .map(
          (v, i) =>
            `${i + 1}. ${v.name} (Age: ${v.age}) – Location: ${v.location}, Status: ${v.status}`
        )
        .join("\n")
    );
  }

  // Resources list (emergency items)
  if (action === "list_resources") {
    if (!Array.isArray(data) || data.length === 0) {
      return "No resources available.";
    }
    const typeName = (t) => ({ first_aid: "First Aid Box", food: "Food", water: "Water", clothes: "Clothes", medical: "Medical Kit", ambulance: "Ambulance" }[t] || t);
    return (
      "Emergency Resources:\n" +
      data
        .map(
          (r, i) =>
            `${i + 1}. #${r.id} – ${typeName(r.type)} (${r.available ? "available" : "in use"})`
        )
        .join("\n")
    );
  }

  // Disaster priority list (max-heap)
  if (action === "list_priority") {
    if (!Array.isArray(data) || data.length === 0) {
      return "No disasters in priority queue.";
    }
    return (
      "Disaster Priority List (highest severity first):\n" +
      data
        .map(
          (d, i) =>
            `${i + 1}. #${d.id} – ${d.type} at ${d.location}, severity ${d.severity}, status: ${d.status}`
        )
        .join("\n")
    );
  }

  // Greedy resource allocation summary
  if (action === "allocate_resources") {
    if (!Array.isArray(data) || data.length === 0) {
      return "No disasters or resources to allocate.";
    }
    return (
      "Greedy Resource Allocation (one resource per disaster, highest severity first):\n" +
      data
        .map(
          (d, i) =>
            `${i + 1}. Disaster #${d.disasterId} (${d.type}) assigned ${d.resourcesAssigned} resource(s)`
        )
        .join("\n")
    );
  }

  // Shortest path via Dijkstra – distance + path
  if (action === "shortest_path") {
    if (!data || data.success === false) {
      return "No rescue path available between the selected locations.";
    }
    const dist = data.distance;
    const path = Array.isArray(data.path) ? data.path.join(" -> ") : "";
    return `Shortest rescue path distance: ${dist} m.\nPath: ${path}`;
  }

  // BFS / DFS traversals
  if (action === "bfs_demo") {
    if (!Array.isArray(data) || data.length === 0) {
      return "BFS traversal: (no nodes).";
    }
    return "BFS traversal from BaseCamp: " + data.join(" -> ");
  }
  if (action === "dfs_demo") {
    if (!Array.isArray(data) || data.length === 0) {
      return "DFS traversal: (no nodes).";
    }
    return "DFS traversal from BaseCamp: " + data.join(" -> ");
  }

  // Fallback
  return JSON.stringify(data, null, 2);
}

// ----- Event bindings -----

document.getElementById("disasterForm").addEventListener("submit", (e) => {
  e.preventDefault();
  const form = e.target;
  const type = form.type.value.trim();
  const location = form.location.value.trim();
  const severity = parseInt(form.severity.value, 10) || 1;
  postAction("add_disaster", { type, location, severity });
  form.reset();
});

document.getElementById("victimForm").addEventListener("submit", (e) => {
  e.preventDefault();
  const form = e.target;
  const name = form.name.value.trim();
  const age = parseInt(form.age.value, 10) || 0;
  const location = form.location.value.trim();
  const status = form.status.value.trim() || "injured";
  postAction("add_victim", { name, age, location, status });
  form.reset();
});

document.getElementById("resourceForm").addEventListener("submit", (e) => {
  e.preventDefault();
  const form = e.target;
  const type = form.type.value;
  postAction("add_resource", { type }).then(() => postAction("list_resources", {}));
});

document.getElementById("btnClearDisasters").addEventListener("click", () => {
  postAction("clear_disasters", {}).then(() => postAction("list_priority", {}));
});

document.getElementById("btnClearVictims").addEventListener("click", () => {
  postAction("clear_victims", {}).then(() => postAction("list_victims", {}));
});

document.getElementById("btnPriority").addEventListener("click", () => {
  postAction("list_priority", {});
});

document.getElementById("btnResources").addEventListener("click", () => {
  postAction("list_resources", {});
});

document.getElementById("btnVictims").addEventListener("click", () => {
  postAction("list_victims", {});
});

document.getElementById("btnAllocate").addEventListener("click", () => {
  postAction("allocate_resources", {});
});

document.getElementById("btnShortestPath").addEventListener("click", () => {
  // Demo: src=0 (BaseCamp), dest=3 (Hospital)
  postAction("shortest_path", { src: 0, dest: 3 });
});

document.getElementById("btnBfs").addEventListener("click", () => {
  postAction("bfs_demo", {});
});

document.getElementById("btnDfs").addEventListener("click", () => {
  postAction("dfs_demo", {});
});

