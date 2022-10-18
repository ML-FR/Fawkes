"""/***************************************************************************
 *  TrainingClipsWorld.py -
 *
 *  Created:
 *  Copyright
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
  *  it under the terms of the GNU General Public License as published by
  *  the Free Software Foundation; either version 2 of the License, or
  *  (at your option) any later version.
  *
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *  GNU Library General Public License for more details.
  *
  *  Read the full text in the LICENSE.GPL file in the doc directory.
  */"""

import sys
from tabnanny import verbose
import numpy as np
import os
import imageio
import gym
import pddlgym

sys.path.append("~/fawkes/src/plugins/clips-gym/")
# Wrapper for PDDLGym for discrete action and observation space
#from PDDLExtension import BlocksWorld, LiteralSpace2, LiteralActionWrapper, LiteralObsWrapper
from ClipsWorld import ClipsWorld#, LiteralSpace2, LiteralActionWrapper, LiteralObsWrapper



#Action Masking
from sb3_contrib.common.maskable.policies import MaskableActorCriticPolicy
from sb3_contrib.common.wrappers import ActionMasker
from sb3_contrib.ppo_mask import MaskablePPO
from stable_baselines3 import PPO
from stable_baselines3.ppo.policies import MlpPolicy
""" 
def env_creator(env_name, dir_path, render):
    domain_file = os.path.join(dir_path, "{}.pddl".format(env_name))

    #gym_name = env_name.capitalize()
    problem_dirname = env_name
    problem_dir = os.path.join(dir_path, problem_dirname)

    print("Load domain file from: {}\nand problem file: {}".format(domain_file, problem_dir))

    #env = BlocksWorld(domain_file, problem_dir, render='blocks_render')#,render) #, seed=0)
    env = ClipsWorld(domain_file, problem_dir, render='blocks_render')#,render) #, seed=0)
    
    env.fix_problem_index(0)
    env = LiteralObsWrapper(LiteralActionWrapper(env)) 
    print(env)
    return env """

# Returns the action mask for the current env. 
def mask_fn(env: gym.Env) -> np.ndarray:
    valid_actions = np.zeros((env.action_space.n), dtype=int)
    for i in range(0, env.action_space.n):
        #check if action is valid
        a = env.env.actions[i]        
        s = env.env.unwrapped._state       
        v = env.env.unwrapped._action_valid_test(s,a)
       
        if v:
            valid_actions[i]=1
    #print("Valid Actions: ", valid_actions)
    return valid_actions

def evaluate(model, num_episodes=5):
    """
    Evaluate a RL agent
    :param model: (BaseRLModel object) the RL Agent
    :param num_episodes: (int) number of episodes to evaluate it
    :return: (float) Mean reward for the last num_episodes
    """
    # This function will only work for a single Environment
    print("ClipsWorld start evaluate")
    env = model.get_env()
    all_episode_rewards = []
    for i in range(num_episodes):
        episode_rewards = []
        done = False
        obs = env.reset()
        print("ClipsWorld: evaluate: obs: ", obs)
        while not done:
            # _states are only useful when using LSTM policies
            print("ClipsWorld: evaluate before predict")
            action, _states = model.predict(obs)
            print("ClipsWorld: evaluate after predict ",action)
            print(_states)
            # here, action, rewards and dones are arrays
            # because we are using vectorized env
            print("ClipsWorld: before step")
            obs, reward, done, info = env.step(action)
            print("ClipsWorld: after step: ",obs, reward, done, info)
            episode_rewards.append(reward)

        all_episode_rewards.append(sum(episode_rewards))

    mean_episode_reward = np.mean(all_episode_rewards)
    print("Mean reward:", mean_episode_reward, "Num episodes:", num_episodes)

    return mean_episode_reward

if __name__ == '__main__':
    render = None
 #   env_name = "blockstower"
#    dir_path = "~/fawkes/src/plugins/rl-test/rl-agent"
    print("In Training script")
    #print("Script: Config values for env_name and path: " + env_name + " " + dir_path)
    #print("Script: Config values for path: " + dir_path)
    print("Script: Creating env")
    
    # action_space = ['TOWER-C1#a#b', 'TOWER-C1#a#c', 'TOWER-C1#a#d' , 'TOWER-C1#a#e']
    # print(action_space)

    # obs_space = ['clear(a)','clear(b)','clear(c)','clear(d)','clear(e)'\
    #     ,'handempty(robo1)','handfull(robo1)','holding(a)','holding(b)','holding(c)','holding(d)','holding(e)'\
    #     ,'on(a,b)','on(a,c)','on(a,d)','on(a,e)','on(b,a)','on(b,c)'\
    #     ,'on(b,d)','on(b,e)','on(c,a)','on(c,b)','on(c,d)','on(c,e)'\
    #     ,'on(d,a)','on(d,b)','on(d,c)','on(d,e)','on(e,a)','on(e,b)'\
    #     ,'on(e,c)','on(e,d)','ontable(a)', 'ontable(b)', 'ontable(c)', 'ontable(d)', 'ontable(e)']
    #print(obs_space)
    
    #env = env_creator(env_name, dir_path, render)
    env = ClipsWorld()#(action_space, obs_space)# render='blocks_render')#,render) #, seed=0)
    
    #print("Env {} created".format(env_name))
    #obs = env.reset()
    
    print("Observations: ", env.obs_dict)
    print("Observation space: ",env.observation_space)
    print("Action space: ",env.action_space)
    print("Actions: ", env.action_dict)

    #Mask environment
    #env = ActionMasker(env, mask_fn)  # Wrap to enable masking

    # MaskablePPO behaves the same as SB3's PPO unless the env is wrapped
    # with ActionMasker. If the wrapper is detected, the masks are automatically
    # retrieved and used when learning. Note that MaskablePPO does not accept
    # a new action_mask_fn kwarg, as it did in an earlier draft.
    #model = MaskablePPO(MaskableActorCriticPolicy, env, verbose=1)
    model = PPO(MlpPolicy, env, verbose=0)

    timesteps=5
    print("Start trainig the rl agent - for {} timesteps".format(timesteps))
    #model.learn(total_timesteps=timesteps)
    # Random Agent, before training
    mean_reward_before_train = evaluate(model)
    print("Mean reward: ",mean_reward_before_train)

    print("Finished training the rl agent")
    print ("Saving the agent at: ", file_name)
    model.save(file_name)

