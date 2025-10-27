local M = {}
M.radius = 96
M.angvel = 0.8
M.follow_player = false

function M.update(self, state, ctx)
  state.t = (state.t or 0) + ctx.dt
  state.phase = state.phase or 0

  local cx, cy
  if M.follow_player then
    cx, cy = ctx.player_x, ctx.player_y
  else
    state.cx = state.cx or self.x
    state.cy = state.cy or self.y
    cx, cy = state.cx, state.cy
  end

  local a = state.phase + state.t * M.angvel
  local x = cx + math.cos(a) * M.radius
  local y = cy + math.sin(a) * M.radius

  local k = 0.30
  self.vx = self.vx + (x - self.x) * k
  self.vy = self.vy + (y - self.y) * k

  self.x = self.x + self.vx * ctx.dt
  self.y = self.y + self.vy * ctx.dt
  return self
end

return M
