import unittest
import sys
from random import choice
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs
from math import sin

class TestTuner(unittest.TestCase):
  def create_tuning_problem(self):
    cs = ccs.ConfigurationSpace(name = "cspace")
    h1 = ccs.NumericalHyperparameter(lower = -5.0, upper = 5.0)
    h2 = ccs.NumericalHyperparameter(lower = -5.0, upper = 5.0)
    h3 = ccs.NumericalHyperparameter(lower = -5.0, upper = 5.0)
    cs.add_hyperparameters([h1, h2, h3])
    os = ccs.ObjectiveSpace(name = "ospace")
    v1 = ccs.NumericalHyperparameter(lower = float('-inf'), upper = float('inf'))
    v2 = ccs.NumericalHyperparameter(lower = float('-inf'), upper = float('inf'))
    os.add_hyperparameters([v1, v2])
    e1 = ccs.Variable(hyperparameter = v1)
    e2 = ccs.Variable(hyperparameter = v2)
    os.add_objectives( [e1, e2] )
    return (cs, os)

  def test_create_random(self):
    (cs, os) = self.create_tuning_problem()
    t = ccs.RandomTuner(name = "tuner", configuration_space = cs, objective_space = os)
    t2 = ccs.Object.from_handle(t.handle)
    self.assertEqual("tuner", t.name)
    self.assertEqual(ccs.TUNER_RANDOM, t.type)
    func = lambda x, y, z: [(x-2)*(x-2), sin(z+y)]
    evals = [ccs.Evaluation(objective_space = os, configuration = c, values = func(*(c.values))) for c in t.ask(100)]
    t.tell(evals)
    hist = t.history
    self.assertEqual(100, len(hist))
    evals = [ccs.Evaluation(objective_space = os, configuration = c, values = func(*(c.values))) for c in t.ask(100)]
    t.tell(evals)
    hist = t.history
    self.assertEqual(200, len(hist))
    optims = t.optimums
    objs = [x.objective_values for x in optims]
    objs.sort(key = lambda x: x[0])
    # assert pareto front
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t.suggest in [x.configuration for x in optims])

  def test_user_defined(self):
    history = []
    optimums = []

    def delete(tuner):
      return None

    def ask(tuner, count):
      if count is None:
        return (None, 1)
      else:
        cs = tuner.configuration_space
        return (cs.samples(count), count)

    def tell(tuner, evaluations):
      nonlocal history
      nonlocal optimums
      history += evaluations
      for e in evaluations:
        discard = False
        new_optimums = []
        for o in optimums:
          if discard:
            new_optimums.append(o)
          else:
            c = e.compare(o)
            if c == ccs.EQUIVALENT or c == ccs.WORSE:
              discard = True
              new_optimums.append(o)
            elif c == ccs.NOT_COMPARABLE:
              new_optimums.append(o)
        if not discard:
          new_optimums.append(e)
        optimums = new_optimums
      return None

    def get_history(tuner):
      return history

    def get_optimums(tuner):
      return optimums

    def suggest(tuner):
      if not optimums:
        return ask(tuner, 1)
      else:
        return choice(optimums).configuration

    (cs, os) = self.create_tuning_problem()
    t = ccs.UserDefinedTuner(name = "tuner", configuration_space = cs, objective_space = os, delete = delete, ask = ask, tell = tell, get_optimums = get_optimums, get_history = get_history, suggest = suggest)
    t2 = ccs.Object.from_handle(t.handle)
    self.assertEqual("tuner", t.name)
    self.assertEqual(ccs.TUNER_USER_DEFINED, t.type)
    self.assertEqual(cs.handle.value, t.configuration_space.handle.value)
    self.assertEqual(os.handle.value, t.objective_space.handle.value)
    func = lambda x, y, z: [(x-2)*(x-2), sin(z+y)]
    evals = [ccs.Evaluation(objective_space = os, configuration = c, values = func(*(c.values))) for c in t.ask(100)]
    t.tell(evals)
    hist = t.history
    self.assertEqual(100, len(hist))
    evals = [ccs.Evaluation(objective_space = os, configuration = c, values = func(*(c.values))) for c in t.ask(100)]
    t.tell(evals)
    hist = t.history
    self.assertEqual(200, len(hist))
    optims = t.optimums
    objs = [x.objective_values for x in optims]
    objs.sort(key = lambda x: x[0])
    # assert pareto front
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t.suggest in [x.configuration for x in optims])


if __name__ == '__main__':
    unittest.main()

