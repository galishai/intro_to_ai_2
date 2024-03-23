import time

from Agent import Agent, AgentGreedy
from WarehouseEnv import WarehouseEnv, manhattan_distance
import random

R0 = 0
R1 = 1


# TODO: section a : 3
def smart_heuristic(env: WarehouseEnv, robot_id: int):
    robot = env.get_robot(robot_id)
    other_robot = env.get_robot((robot_id + 1) % 2)
    charge1 = env.charge_stations[0].position
    charge2 = env.charge_stations[1].position

    way1 = manhattan_distance(robot.position, env.packages[0].position) + manhattan_distance(env.packages[0].position,
                                                                                             env.packages[
                                                                                                 0].destination) + min(
        manhattan_distance(env.packages[0].destination, env.charge_stations[0].position),
        manhattan_distance(env.packages[0].destination, env.charge_stations[1].position))
    way2 = manhattan_distance(robot.position, env.packages[1].position) + manhattan_distance(env.packages[1].position,
                                                                                             env.packages[
                                                                                                 1].destination) + min(
        manhattan_distance(env.packages[0].destination, env.charge_stations[0].position),
        manhattan_distance(env.packages[0].destination, env.charge_stations[1].position))
    to_package_1 = manhattan_distance(robot.position, env.packages[0].position)
    to_package_2 = manhattan_distance(robot.position, env.packages[1].position)
    to_charge_1 = manhattan_distance(robot.position, env.charge_stations[0].position)
    to_charge_2 = manhattan_distance(robot.position, env.charge_stations[1].position)

    expected_gain_1 = 2 * manhattan_distance(env.packages[0].position, env.packages[0].destination)
    expected_gain_2 = 2 * manhattan_distance(env.packages[1].position, env.packages[1].destination)

    if robot.package is not None:
        expected_gain = 2 * manhattan_distance(robot.package.position, robot.package.destination)
        if manhattan_distance(robot.position, robot.package.destination) + min(
                manhattan_distance(robot.package.destination, charge1),
                manhattan_distance(robot.package.destination, charge2)) > robot.battery - 3 and other_robot.battery > 0:
            return 1000 * ((robot.credit - other_robot.credit)) + 200 * (robot.battery - other_robot.battery) + 1 / (
                    1 / (min(to_charge_1, to_charge_2) + 1)) + (expected_gain) / (robot.battery ** 2 + 1)
        else:
            return 1000 * ((robot.credit) - other_robot.credit) + 1 / (
                (manhattan_distance(robot.position, robot.package.destination) + 1)) + 200 * (
                    robot.battery - other_robot.battery) + (expected_gain) / (robot.battery ** 2 + 1)

    if (expected_gain_1 - way1) > (expected_gain_2 - way2) and robot.battery >= way1 and expected_gain_1 > 0:
        return 1000 * ((robot.credit) - other_robot.credit) + (1 / (to_package_1 + 1)) + 200 * (
                robot.battery - other_robot.battery)
    else:
        return 1000 * ((robot.credit) - other_robot.credit) + (1 / (to_package_2 + 1)) + 200 * (
                robot.battery - other_robot.battery)


class AgentGreedyImproved(AgentGreedy):
    def heuristic(self, env: WarehouseEnv, robot_id: int):
        return smart_heuristic(env, robot_id)


class AgentMinimax(Agent):
    # TODO: section b : 1
    def run_step(self, env: WarehouseEnv, agent_id, time_limit):
        finish = time.time() + time_limit * 0.95
        action = None
        depth = 0
        try:
            while True:
                if time.time() >= finish:
                    raise Exception("Time limit reached!")
                action = rb_minmax(env, agent_id, finish, None, depth, agent_id)[0]
                depth += 1
        except:
            return action


def rb_minmax(env: WarehouseEnv, agent_id, finish_time, current_action, depth, turn):
    if time.time() >= finish_time:
        raise Exception("Time limit reached!")
    if env.done():
        r_curr = env.get_robot(agent_id)
        r_other = env.get_robot(1 - agent_id)
        return current_action, r_curr.credit - r_other.credit
    if depth == 0:
        if turn == agent_id:
            return current_action, smart_heuristic(env, agent_id)
        else:
            return current_action, smart_heuristic(env, agent_id)
    operators = env.get_legal_operators(turn)
    robot = env.get_robot(agent_id)
    other_robot = env.get_robot((agent_id + 1) % 2)
    if robot.credit > other_robot.credit and other_robot.battery == 0 and 'charge' in operators:
        operators.remove('charge')
    children = [env.clone() for _ in operators]
    action = operators[0]
    if turn == agent_id:
        max_val = float('-inf')
        for child, op in zip(children, operators):
            child.apply_operator(turn, op)
        while children:
            children_heuristics = [smart_heuristic(c, agent_id) for c in children]
            max_heuristic = max(children_heuristics)
            index_selected = children_heuristics.index(max_heuristic)
            c_val = \
                rb_minmax(children[index_selected], agent_id, finish_time, operators[index_selected], depth - 1,
                          1 - turn)[
                    1]
            if c_val > max_val:
                max_val = c_val
                action = operators[index_selected]
            children.remove(children[index_selected])
            operators.remove(operators[index_selected])
        return action, max_val
    else:  # turn == R1
        min_val = float('inf')
        for child, op in zip(children, operators):
            child.apply_operator(turn, op)
            c_val = rb_minmax(child, agent_id, finish_time, op, depth - 1, 1 - turn)[1]
            if c_val < min_val:
                min_val = c_val
                action = op
        return action, min_val


class AgentAlphaBeta(Agent):
    # TODO: section c : 1
    def run_step(self, env: WarehouseEnv, agent_id, time_limit):
        finish = time.time() + time_limit * 0.95
        action = None
        depth = 0
        try:
            while True:
                if time.time() >= finish:
                    raise Exception("Time limit reached!")
                alpha = float('-inf')
                beta = float('inf')
                action = alpha_beta_minmax(env, agent_id, finish, None, depth, agent_id, alpha, beta)[0]
                depth += 1
        except:
            return action


def alpha_beta_minmax(env: WarehouseEnv, agent_id, finish_time, current_action, depth, turn, alpha, beta):
    if time.time() >= finish_time:
        raise Exception("Time limit reached!")
    if env.done() or depth == 0:
        return current_action, smart_heuristic(env, agent_id)
    operators = env.get_legal_operators(turn)
    children = [env.clone() for _ in operators]
    for child, op in zip(children, operators):
        child.apply_operator(turn, op)
    action = operators[0]

    if turn == agent_id:
        max_val = float('-inf')
        while children:
            children_heuristics = [smart_heuristic(c, agent_id) for c in children]
            max_heuristic = max(children_heuristics)
            index_selected = children_heuristics.index(max_heuristic)
            c_val = \
                alpha_beta_minmax(children[index_selected], agent_id, finish_time, operators[index_selected], depth - 1,
                                  1 - turn, alpha, beta)[1]
            if c_val > max_val:
                max_val = c_val
                action = operators[index_selected]
            children.remove(children[index_selected])
            operators.remove(operators[index_selected])
            alpha = max(alpha, max_val)
            if max_val >= beta:
                return action, float('inf')
        return action, max_val
    else:  # turn == R1
        min_val = float('inf')
        for child, op in zip(children, operators):
            c_val = alpha_beta_minmax(child, agent_id, finish_time, op, depth - 1, 1 - turn, alpha, beta)[1]
            if c_val < min_val:
                min_val = c_val
                action = op
            beta = min(beta, min_val)
            if min_val <= alpha:
                return action, float('-inf')
        return action, min_val


class AgentExpectimax(Agent):
    # TODO: section d : 1
    def run_step(self, env: WarehouseEnv, agent_id, time_limit):
        finish = time.time() + time_limit - 0.1
        action = None
        depth = 0
        try:
            while True:
                if time.time() >= finish:
                    raise Exception("Time limit reached!")
                action = expectimax(env, agent_id, finish, None, depth, agent_id)[0]
                depth += 1
        except:
            return action


def expectimax(env: WarehouseEnv, agent_id, finish_time, current_action, depth, turn):
    if time.time() >= finish_time:
        raise Exception("Time limit reached!")
    if depth == 0 or env.done():
        return current_action, smart_heuristic(env, agent_id)
    operators = env.get_legal_operators(turn)
    children = [env.clone() for _ in operators]
    action = operators[0]
    if turn == agent_id:
        max_val = float('-inf')
        for child, op in zip(children, operators):
            child.apply_operator(turn, op)
            c_val = rb_minmax(child, agent_id, finish_time, op, depth - 1, 1 - turn)[1]
            if c_val > max_val:
                max_val = c_val
                action = op
        return action, max_val
    else:
        p = 1 / len(children)
        exp = 0
        min_val = float('inf')
        for child, op in zip(children, operators):
            child.apply_operator(turn, op)
            c_val = rb_minmax(env, agent_id, finish_time, op, depth - 1, 1 - turn)[1]
            if op == 'move east' or op == 'pick up':
                exp += 2 * p * c_val
            else:
                exp += p * c_val
        return action, exp


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
