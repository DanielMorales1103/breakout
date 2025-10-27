local M = {}
M.speed = 70
M.arrive = 14

local function default_wps(self)
  return {
    { self.x, self.y },
    { self.x + 160, self.y },
    { self.x + 160, self.y + 120 },
    { self.x, self.y + 120 },
  }
end

function M.update(self, state, ctx)
  state.idx = state.idx or 1
  state.wps = state.wps or default_wps(self)

  local wp = state.wps[state.idx]
  local dx, dy = wp[1] - self.x, wp[2] - self.y
  local d = math.max(1e-6, math.sqrt(dx*dx + dy*dy))
  if d < M.arrive then
    state.idx = (state.idx % #state.wps) + 1
    return self
  end

  local nx, ny = dx/d, dy/d
  self.vx = nx * M.speed
  self.vy = ny * M.speed

  self.x = self.x + self.vx * ctx.dt
  self.y = self.y + self.vy * ctx.dt
  return self
end

return M
