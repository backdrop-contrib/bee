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

4. An optional step to ensure you can easily run `bee` on the site from the app root (or other folders that aren't in the `webroot` is to replace the tooling with this (where `docroot` is the value of `webroot` in the recipe; adjust if you have a different `webroot`):
```yaml
tooling:
  bee:
    service: appserver
    cmd: /usr/local/bin/bee/bee.php --root=/app/docroot
```

## Usage
Use Bee by typing `lando bee ...`