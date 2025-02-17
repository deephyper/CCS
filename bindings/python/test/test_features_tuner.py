import unittest
import sys
sys.path.insert(1, '.')
sys.path.insert(1, '..')
import cconfigspace as ccs
from math import sin
from random import choice

class TestFeaturesTuner(unittest.TestCase):
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
    fs = ccs.FeaturesSpace(name = "fspace")
    f1 = ccs.CategoricalHyperparameter(values = [True, False])
    fs.add_hyperparameter(f1)
    return (cs, fs, os)

  def test_create_random(self):
    (cs, fs, os) = self.create_tuning_problem()
    t = ccs.RandomFeaturesTuner(name = "tuner", configuration_space = cs, features_space = fs, objective_space = os)
    t2 = ccs.Object.from_handle(t.handle)
    self.assertEqual("tuner", t.name)
    self.assertEqual(ccs.FEATURES_TUNER_RANDOM, t.type)
    func = lambda x, y, z: [(x-2)*(x-2), sin(z+y)]
    features_on = ccs.Features(features_space = fs, values = [True])
    features_off = ccs.Features(features_space = fs, values = [False])
    evals = [ccs.FeaturesEvaluation(objective_space = os, configuration = c, features = features_on, values = func(*(c.values))) for c in t.ask(features_on, 50)]
    t.tell(evals)
    evals = [ccs.FeaturesEvaluation(objective_space = os, configuration = c, features = features_off, values = func(*(c.values))) for c in t.ask(features_off, 50)]
    t.tell(evals)
    hist = t.history()
    self.assertEqual(100, len(hist))
    evals = [ccs.FeaturesEvaluation(objective_space = os, configuration = c, features = features_on, values = func(*(c.values))) for c in t.ask(features_on, 100)]
    t.tell(evals)
    self.assertEqual(200, t.history_size())
    self.assertEqual(150, t.history_size(features = features_on))
    self.assertEqual(50, t.history_size(features = features_off))
    optims = t.optimums(features = features_on)
    objs = [x.objective_values for x in optims]
    objs.sort(key = lambda x: x[0])
    # assert pareto front
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t.suggest(features_on) in [x.configuration for x in optims])
    optims = t.optimums(features = features_off)
    objs = [x.objective_values for x in optims]
    objs.sort(key = lambda x: x[0])
    # assert pareto front
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t.suggest(features_off) in [x.configuration for x in optims])


  def test_user_defined(self):
    history = []
    optimums = []

    def delete(tuner):
      return None

    def ask(tuner, features, count):
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

    def get_history(tuner, features):
      if features is not None:
        return list(filter(lambda e: e.features == features, history))
      else:
        return history

    def get_optimums(tuner, features):
      if features is not None:
        return list(filter(lambda e: e.features == features, optimums))
      else:
        return optimums

    def suggest(tuner, features):
      optis = list(filter(lambda e: e.features == features, optimums))
      if not optis:
        return ask(tuner, features, 1)
      else:
        return choice(optis).configuration

    (cs, fs, os) = self.create_tuning_problem()
    t = ccs.UserDefinedFeaturesTuner(name = "tuner", configuration_space = cs, features_space = fs, objective_space = os, delete = delete, ask = ask, tell = tell, get_optimums = get_optimums, get_history = get_history, suggest = suggest)
    t2 = ccs.Object.from_handle(t.handle)
    self.assertEqual("tuner", t.name)
    self.assertEqual(ccs.FEATURES_TUNER_USER_DEFINED, t.type)
    self.assertEqual(cs.handle.value, t.configuration_space.handle.value)
    self.assertEqual(fs.handle.value, t.features_space.handle.value)
    self.assertEqual(os.handle.value, t.objective_space.handle.value)
    func = lambda x, y, z: [(x-2)*(x-2), sin(z+y)]
    features_on = ccs.Features(features_space = fs, values = [True])
    features_off = ccs.Features(features_space = fs, values = [False])
    evals = [ccs.FeaturesEvaluation(objective_space = os, configuration = c, features = features_on, values = func(*(c.values))) for c in t.ask(features_on, 50)]
    t.tell(evals)
    evals = [ccs.FeaturesEvaluation(objective_space = os, configuration = c, features = features_off, values = func(*(c.values))) for c in t.ask(features_off, 50)]
    t.tell(evals)
    hist = t.history()
    self.assertEqual(100, len(hist))
    evals = [ccs.FeaturesEvaluation(objective_space = os, configuration = c, features = features_on, values = func(*(c.values))) for c in t.ask(features_on, 100)]
    t.tell(evals)
    self.assertEqual(200, t.history_size())
    self.assertEqual(150, t.history_size(features = features_on))
    self.assertEqual(50, t.history_size(features = features_off))
    optims = t.optimums(features = features_on)
    objs = [x.objective_values for x in optims]
    objs.sort(key = lambda x: x[0])
    # assert pareto front
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t.suggest(features_on) in [x.configuration for x in optims])
    optims = t.optimums(features = features_off)
    objs = [x.objective_values for x in optims]
    objs.sort(key = lambda x: x[0])
    # assert pareto front
    self.assertTrue(all(objs[i][1] >= objs[i+1][1] for i in range(len(objs)-1)))
    self.assertTrue(t.suggest(features_off) in [x.configuration for x in optims])


if __name__ == '__main__':
    unittest.main()

