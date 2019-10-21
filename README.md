# Backdrop Console

`Backdrop Console` is a command line utility for Backdrop CMS developers. It is
aimed at speeding up your development cycles.

## Warning

- Not Windows compatible
- Possible API changes
- Under active development

## Installation

- Clone this repository to your computer (your home directory is a good place):
  - `git clone https://github.com/backdrop-contrib/b.git`
- Make `b.php` executable:
  - `chmod +x b.php`
- Put a symlink to `b.php` in your $PATH:
  - `sudo ln -s /home/[username]/b/b.php /usr/local/bin/b`
- Test to make sure it works:
  - Simply type `b` (you should see a list of available commands printed out)

## Usage

Please use `b help` to see full list of available commands.

At the moment the following commands are available:

```
 cache-clear        Clear a specific cache, or all Backdrop caches.
                    aliases: cc
 config-get         Show config settings.
 config-list        Show a list of configs.
 config-set         Set config settings.
 core-status        Provides a birds-eye view of the current Backdrop installation, if any.
                    This is test example how to add your own callback on command.
                    aliases: status, st
 cron               Process cron.
 help               Display help.
 pm-disable         Disable one or more modules.
                    Disable one or more themes.
                    aliases: dis
 pm-download        Download contrib package.
                    aliases: dl
 pm-enable          Enable one or more modules.
                    Enable one or more themes.
                    aliases: en
 pm-list            Show a list of available extensions (modules, layouts or themes).
                    aliases: pml
 pm-uninstall       Uninstall one or more modules.
                    aliases: pmu
 pm-info            Show detailed info for one or more extensions (modules or themes).
                    aliases: pmi
 site-install       Install Backdrop along with modules/themes/configuration using the specified install profile.
                    aliases: si
 updatedb           Apply any database updates required (as with running update.php).
                    aliases: updb
 updatedb-status    Show a list of updates.
                    aliases: updbst
 watchdog-show      Show dblog messages.
                    aliases: wd-show, ws
```

To see command details, use `b help command_name`:

```
# b help site-install
Extension site_install
Install Backdrop along with modules/themes/configuration using the specified install profile.

Options:
 --root         : Set the working directory for the script to the specified path. Required if running this script from
                :  a directory other than the Backdrop root.
 --db-url       : A Drupal 6 style database URL. Only required for initial install - not re-install.
                : example: mysql://root:pass@host/db
 --db-prefix    : An optional table prefix to use for initial install.  Can be a key-value array of tables/prefixes in
                :  a drushrc file (not the command line).
 --account-name : uid1 name. Defaults to admin
 --account-pass : uid1 pass. Defaults to a randomly generated password. If desired, set a fixed password in drushrc.ph
                : p.
 --account-mail : uid1 email. Defaults to admin@example.com
 --locale       : A short language code. Sets the default site language. Language files must already be present. You m
                : ay use download command to get them.
                : example: en-GB
 --clean-url    : Defaults to 1
 --site-name    : Defaults to Site-Install
 --site-mail    : From: for system mailings. Defaults to admin@example.com

Arguments:
 profile   : The install profile you wish to run. Default is "standard"
 key=value : Any additional settings you wish to pass to the profile. The key is in the form [form name].[parameter na
           : me]

Aliases: si
```

## Maintainers

  * Gor Martsen (https://github.com/Gormartsen)
  * Geoff St. Pierre (https://github.com/serundeputy)
  * John Franklin (https://github.com/jlfranklin)
  * Peter Anderson (https://github.com/BWPanda)

## Thanks to drush

This module was inspired by (and some code and logic was copied from)
[drush](https://github.com/drush-ops/drush).

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.

