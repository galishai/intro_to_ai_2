import time

from Agent import Agent, AgentGreedy
from WarehouseEnv import WarehouseEnv, manhattan_distance
import random

R1 = 0
R2 = 1


# TODO: section a : 3
def smart_heuristic(env: WarehouseEnv, robot_id: int):
    pass


class AgentGreedyImproved(AgentGreedy):
    def heuristic(self, env: WarehouseEnv, robot_id: int):
        return smart_heuristic(env, robot_id)


class AgentMinimax(Agent):
    # TODO: section b : 1
    def run_step(self, env: WarehouseEnv, agent_id, time_limit):
        finish = time.time() + time_limit - 0.1
        action = None
        depth = 0
        turn = 0
        try:
            while True:
                if time.time() >= finish:
                    raise Exception("Time limit reached!")
                action = self.rb_minmax(WarehouseEnv, agent_id, finish, None, depth, 0)
        except:
            return action


def rb_minmax(env: WarehouseEnv, agent_id, finish, current_action, depth, turn):
    if time.time() >= finish:
        raise Exception("Time limit reached!")
    if depth == 0 or WarehouseEnv.done():
        return current_action, smart_heuristic(agent_id)
    operators = WarehouseEnv.get_legal_operators(turn)
    action = operators[0]
    if turn == R1:
        max_val = float('-inf')
        for move in operators:
            c_val = rb_minmax(WarehouseEnv, agent_id, finish, move, depth - 1, R2)[1]
            if c_val > max_val:
                max_val = c_val
                action = move
        return action, max_val
    else:  # turn == R2
        min_val = float('inf')
        for move in operators:
            c_val = rb_minmax(WarehouseEnv, agent_id, finish, move, depth - 1, R1)[1]
            if c_val < min_val:
                min_val = c_val
                action = move
        return action, min_val


class AgentAlphaBeta(Agent):
    # TODO: section c : 1
    def run_step(self, env: WarehouseEnv, agent_id, time_limit):
        raise NotImplementedError()


class AgentExpectimax(Agent):
    # TODO: section d : 1
    def run_step(self, env: WarehouseEnv, agent_id, time_limit):
        raise NotImplementedError()


# here you can check specific paths to get to know the environment
class AgentHardCoded(Agent):
    def __init__(self):
        self.step = 0
        # specifiy the path you want to check - if a move is illegal - the agent will choose a random move
        self.trajectory = ["move north", "move east", "move north", "move north", "pick_up", "move east", "move east",
                           "move south", "move south", "move south", "move south", "drop_off"]

    def run_step(self, env: WarehouseEnv, robot_id, time_limit):
        if self.step == len(self.trajectory):
            return self.run_random_step(env, robot_id, time_limit)
        else:
            op = self.trajectory[self.step]
            if op not in env.get_legal_operators(robot_id):
                op = self.run_random_step(env, robot_id, time_limit)
            self.step += 1
            return op

    def run_random_step(self, env: WarehouseEnv, robot_id, time_limit):
        operators, _ = self.successors(env, robot_id)

        return random.choice(operators)
