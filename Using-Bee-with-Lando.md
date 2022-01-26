Bee can be used with your existing [Lando](https://lando.dev/) setup:

## Installation
1. Add some build steps that download and install Bee:
  ```yaml
  services:
    appserver:
      build:
        - wget -qO bee.zip https://github.com/backdrop-contrib/bee/archive/1.x-1.x.zip
        - unzip -q bee.zip && rm bee.zip
        - mv bee-1.x-1.x /usr/local/bin/bee
  ```

2. Add a tooling command for `bee`:
  ```yaml
  tooling:
    bee:
      service: appserver
      cmd: /usr/local/bin/bee/bee.php
  ```

3. Rebuild Lando to make the above changes take effect: `lando rebuild`

## Usage
Use Bee by typing `lando bee ...`