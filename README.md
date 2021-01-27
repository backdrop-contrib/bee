Backdrop Console
================

Backdrop Console (a.k.a. `b`) is a command line utility for Backdrop CMS. It
includes commands that allow developers to interact with Backdrop sites,
performing actions like:

- Running cron
- Clearing caches
- Downloading and installing Backdrop
- Downloading, enabling and disabling projects
- Viewing information about a site and/or available projects

Please note that `b` is under active development and, as such, breaking changes
may occur. Please see the release notes for a list of any major changes between
versions. Also note that `b` is not (yet?) compatible with Microsoft Windows.

Installation
------------

- Download (or clone) `b` to your computer/server.  
  This will create a folder called `b` with lots of files inside. Your home
  directory is a good location for this folder.

- Make sure `b/b.php` is executable.  
  It should be by default, but it doesn't hurt to double-check.

- Make it easy to run `b` commands.  
  There are two ways to do this:

  1. Put a symlink to `b/b.php` in your `$PATH` (e.g.
     `sudo ln -s /path/to/b/b.php /usr/local/bin/b`)

  2. Make a Bash alias for `b` (e.g. add `alias b='/path/to/b/b.php'` to your
     `.bash_aliases` file)

- Test to make sure it works.  
  Simply type `b` in your terminal and you should see a list of available
  commands displayed.

Issues
------

Bugs and feature requests should be reported in the issue queue:
https://github.com/backdrop-contrib/b/issues.

Current Maintainers
-------------------

- [Peter Anderson](https://github.com/BWPanda)

Credits
-------

- Originally written for Backdrop CMS by
  [Geoff St. Pierre](https://github.com/serundeputy).
- Inspired by [Drush](https://github.com/drush-ops/drush).

License
-------

This project is GPL v2 software.
See the LICENSE.txt file in this directory for complete text.
