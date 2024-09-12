# Bee

<img src="https://raw.githubusercontent.com/backdrop-contrib/bee/1.x-1.x/images/bee.png" align="right" width="150" height="157">

Bee is a command line utility for Backdrop CMS. It includes commands that allow
developers to interact with Backdrop sites, performing actions like:

- Running cron
- Clearing caches
- Downloading and installing Backdrop
- Downloading, enabling and disabling projects
- Viewing information about a site and/or available projects

See the Release notes and the Changelog for details of changes between
versions.

## Requirements
### Operating System

![Static Badge](https://img.shields.io/badge/os%20compatibility-555555?style=flat-square)
![Static Badge](https://img.shields.io/badge/linux-blue?logo=linux&logoColor=white&style=flat-square)
![Static Badge](https://img.shields.io/badge/macOS-blue?logo=apple&logoColor=white&style=flat-square)
![Static Badge](https://img.shields.io/badge/WSL2-blue?logo=linux&logoColor=white&style=flat-square)

- Bee will work in most Linux based environments.
- Bee will NOT work natively in Windows but can be used inside Linux based
virtual environments such as WSL2.
- Bee requires the `mysql` and `mysqldump` executables to be installed for most
database operations (i.e. `db-export`, `db-import`, `db-drop`, `sql` but NOT
`db-query`).

### PHP
![Static Badge](https://img.shields.io/badge/php%20compatibility-555555?logo=php&logoColor=white&style=flat-square)
![Static Badge](https://img.shields.io/badge/8.3-blue?style=flat-square)
![Static Badge](https://img.shields.io/badge/8.2-blue?style=flat-square)
![Static Badge](https://img.shields.io/badge/8.1-blue?style=flat-square)
![Static Badge](https://img.shields.io/badge/8.0-blue?style=flat-square)
![Static Badge](https://img.shields.io/badge/7.4-blue?style=flat-square)

- Bee is tested and works from `7.4` up to `8.3`.

## Installation

- Download (or clone) Bee to your computer/server.  
  This will create a folder called `bee` with lots of files inside. Your home
  directory is a good location for this folder. It is neither required nor
  recommended to place this folder within the Backdrop web root.

- Make sure `bee/bee.php` is executable.  
  It should be by default, but it doesn't hurt to double-check.

- Make it easy to run `bee` commands.  
  There are two ways to do this:

  1. Put a symlink to `bee/bee.php` in your `$PATH` (e.g.
     `sudo ln -s /path/to/bee/bee.php /usr/local/bin/bee`)

  2. Make a Bash alias for `bee` (e.g. add `alias bee='/path/to/bee/bee.php'` to
     your `.bash_aliases` file)

- Test to make sure it works.  
  Simply type `bee` in your terminal and you should see a list of available
  commands displayed.

More advanced/specialised installation instructions can be found in the
[wiki](https://github.com/backdrop-contrib/bee/wiki).

## Extending

Bee can be extended (i.e. custom/additional commands added) by Backdrop contrib
modules or by individual users. See
[API.md](https://github.com/backdrop-contrib/bee/blob/1.x-1.x/API.md) for
details. 

## Issues

Bugs and feature requests should be reported in the issue queue:
https://github.com/backdrop-contrib/bee/issues.

## Current Maintainers

- [Martin Price](https://github.com/yorkshire-pudding) - [System Horizons](https://www.systemhorizons.co.uk)
- Collaboration and co-maintainers welcome!

## Credits

- Originally written for Backdrop CMS by
  [Geoff St. Pierre](https://github.com/serundeputy)  
  (originally called 'Backdrop Console (a.k.a. `b`)').
- Grateful thanks goes to previous maintainers and collaborators who have
helped bring Bee to where it is today. See the 
[list of contributors](https://github.com/backdrop-contrib/bee/graphs/contributors)
for details.
- Inspired by [Drush](https://github.com/drush-ops/drush).
- [Bee icon](https://thenounproject.com/aomam/collection/bee-emoticons-line/?i=2257433)
  by AomAm from [the Noun Project](http://thenounproject.com).

## License

This project is GPL v2 software.
See the LICENSE.txt file in this directory for complete text.
