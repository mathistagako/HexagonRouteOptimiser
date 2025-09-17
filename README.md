# Hexagonal Grid Routing Application

This application simulates a logistics system for a transportation company operating on a hexagonal grid map.  
Its main purpose is to calculate **optimal travel costs** between locations, considering both land movements and optional air routes.

---

## 🌍 Map Structure

- The world is represented as a **rectangular grid of hexagonal tiles**.
- Each hexagon is identified by its coordinates `(x, y)`:
  - `x` = column index (starting from `0`, left → right)
  - `y` = row index (starting from `0`, bottom → top)
- Each hexagon has up to **six neighbors**, except at the map’s borders.

---

## 🚚 Movement and Costs

- Vehicles move from one hexagon to an adjacent one.
- Every hexagon has a **land exit cost**:
  - `0` → cannot be exited (but can still be visited).
  - `1–100` → cost of leaving the hexagon by land.
- **Air routes** can be created or removed:
  - Each air route is **directed** (one-way).
  - A maximum of **5 outgoing routes** is allowed per hexagon.
  - Each route has its own traversal cost.

🔑 **Rules:**
- Moving to an adjacent hexagon by land costs the **exit cost of the current hexagon**.
- Moving via an **air route** ignores the land exit cost of the source and only considers the route cost.

---

## ⚙️ Commands

The program reads commands from **standard input** and writes results to **standard output**.  
Each response ends with a newline character (`\n`).

### `init <columns> <rows>`
Initializes (or resets) the map.  
- All hexagons have a cost of `1`.  
- No air routes exist.  
**Response:** `OK`

---

### `change_cost <x> <y> <v> <radius>`
Updates the costs of all hexagons within distance `<radius>` from `(x, y)`.  
- `v` is an integer between `-10` and `10`, applied with a distance decay.  
- Outgoing air route costs from affected hexagons are also updated.  
**Response:** `OK` if valid, otherwise `KO`.

---

### `toggle_air_route <x1> <y1> <x2> <y2>`
Adds or removes an air route from `(x1, y1)` to `(x2, y2)`.  
- If added, the route cost is the **floor of the average** of:
  - all existing outgoing air route costs from `(x1, y1)`
  - plus its land exit cost.  
- At most **5 outgoing routes** are allowed.  
**Response:** `OK` if successful, otherwise `KO`.

---

### `travel_cost <xs> <ys> <xd> <yd>`
Computes the **minimum travel cost** from source `(xs, ys)` to destination `(xd, yd)`.  
- Destination’s exit cost is ignored.  
- If source = destination → cost = `0`.  
- Returns `-1` if invalid coordinates or unreachable.  

---

## 📌 Example

```txt
init 100 100
→ OK

change_cost 10 20 -10 5
→ OK   # Makes a region non-traversable

travel_cost 0 0 20 0
→ 20   # Sum of land costs

toggle_air_route 0 0 20 0
→ OK   # Adds an air route

travel_cost 0 0 20 0
→ 1    # Air route drastically reduces cost

toggle_air_route 0 0 20 0
→ OK   # Removes the air route

travel_cost 0 0 20 0
→ 20   # Back to land path cost
