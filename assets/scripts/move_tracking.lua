local M = {}
M.turn = 0.15
M.arrive_radius = 32

function M.update(self, state, ctx)
  local px, py = ctx.player_x, ctx.player_y
  local dx, dy = px - self.x, py - self.y
  local d = math.max(1e-6, math.sqrt(dx*dx + dy*dy))
  local nx, ny = dx/d, dy/d

  local speed = ctx.maxSpeed or 60
  if d < M.arrive_radius then
    speed = speed * (d / M.arrive_radius)
  end

  local desired_x, desired_y = nx*speed, ny*speed
  self.vx = self.vx + (desired_x - self.vx) * M.turn
  self.vy = self.vy + (desired_y - self.vy) * M.turn

  self.x = self.x + self.vx * ctx.dt
  self.y = self.y + self.vy * ctx.dt
  return self
end

return M
