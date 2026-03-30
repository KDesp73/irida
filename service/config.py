import yaml
import os

CONFIG_PATH = os.path.join(os.path.dirname(__file__), "config.yml")
with open(CONFIG_PATH, "r") as f:
    CONFIG = yaml.safe_load(f)
