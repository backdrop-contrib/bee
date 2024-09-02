## Command structure

```bash
bee [global-options] <command> [options] [arguments]
```

<details>
<summary>Use within Lando, DDEV or Docksal</summary>

### Lando

```bash
lando bee [global-options] <command> [options] [arguments]
```

### DDEV

```bash
ddev bee [global-options] <command> [options] [arguments]
```

### Docksal

```bash
fin bee [global-options] <command> [options] [arguments]
```

</details>

## Global Options

- `--root`  
  Specify the root directory of the Backdrop installation to use. If not set,
  will try to find the Backdrop installation automatically based on the current
  directory. For example, `bee --root=docroot status`
- `--site`  
  Specify the directory name or URL of the Backdrop site to use (as defined in
  'sites.php'). If not set, will try to find the Backdrop site automatically
  based on the current directory. For example `bee --site=example-a status` or
  `bee --site=www.example-a.com status`
- `--base-url`  
  Specify the base URL of the Backdrop site, such as `https://example.com/backdrop`. May be useful with commands that output URLs to pages on the site.
- `--yes`, `-y`  
  Answer 'yes' to questions without prompting.
- `--debug`, `-d`  
  Enables 'debug' mode, in which 'log' type messages will be displayed (in
  addition to all other messages).

## Commands

### Configuration

#### `config-export`
*Description:* Export config from the site.  
*Aliases:* `cex`, `bcex`  
*Examples:*
- `bee config-export`  
  Exports config (from 'active' to 'staging') for the current site.

#### `config-import`
*Description:* Import config into the site.  
*Aliases:* `cim`, `bcim`  
*Examples:*
- `bee config-import`  
  Imports config (from 'staging' to 'active') for the current site.

#### `config-get`
*Description:* Get the value of a specific config option, or view all the config
options in a given file.  
*Aliases:* `cget`  
*Arguments:*
- `file`  
  The name of the config object to retrieve. This is the name of the config
  file, less the '.json' suffix.
- `option`  
  (optional) The name of the config option within the file to read. This may
  contain periods to indicate levels within the config file. Leave blank to
  display the entire config file.

*Examples:*
- `bee config-get system.core site_name`  
  Get the value of the 'site_name' config option.
- `bee config-get devel.settings`  
  See all the config options for the Devel module.

#### `config-set`
*Description:* Set the value of an option in a config file.  
*Aliases:* `cset`  
*Arguments:*
- `file`  
  The name of the config object to retrieve. This is the name of the config
  file, less the '.json' suffix.
- `option`  
  The name of the config option within the file to set. This may contain periods
  to indicate levels within the config file.
- `value`  
  The value to save in the config file.

*Examples:*
- `bee config-set image.style.thumbnail effects.0.data.width 200`  
  Change the width of the Thumbnail image style.

### Projects
#### `projects`
*Description:* Display information about available projects (modules, themes, layouts).  
*Aliases:* `pml` , `pmi` , `project` , `pm-list` , `pm-info`  
*Arguments:*
- `project` - (optional) The name of a project to display information for. Leave blank to list information about all projects.

*Options:*
- `--type=TYPE` - Restrict list to projects of a given type: 'module', 'theme' or 'layout'.

*Examples:*
- `bee projects webform` - Show information about the Webform module.
- `bee projects --type=layout` - Show information about all available layouts.
- `bee projects` - Show information about all available projects.

#### `download`
*Description:* Download Backdrop contrib projects together with dependencies.  
*Aliases:* `dl` , `pm-download`  
*Arguments:*
- `projects` - One or more contrib projects to download.

*Options:*
- `--github-token` - A Github Personal Access Token (Classic) that can be used to extend the GitHub API rate.
- `--hide-progress`, `-h` - Deprecated, will get removed in a future version.
- `--allow-multisite-copy`, `-f` - Override the check that would prevent the project being downloaded to a multisite site if the project exists in the shared project directory.

*Examples:*
- `bee download webform` - Download the Webform module.
- `bee download simplify thesis bamboo` - Download the Simplify module, Thesis theme, and Bamboo layout.
- `bee --site=site_name download simplify --allow-multisite-copy` - Download an additional copy of the Simplify module into the site_name multisite module folder.

#### `enable`
*Description:* Enable one or more projects (modules, themes, layouts).  
*Aliases:* `en` , `pm-enable`  
*Arguments:*
- `projects` - One or more (space separated) projects to enable.

*Options:*
- `--no-dependency-checking`, `-n` - Disable dependency-checking and enable module(s) regardless. This could cause problems if there are missing dependencies. Use with caution.

*Examples:*  
- `bee enable webform` - Enable the Webform module.
- `bee enable --no-dependency-checking rules` - Enable the Rules module, regardless of whether or not its dependencies are available.
- `bee enable simplify thesis bamboo` - Enable the Simplify module, Thesis theme, and Bamboo layout.

#### `disable`                                   
*Description:* Disable one or more projects (modules, themes, layouts).  
*Aliases:* `dis` , `pm-disable`  
*Arguments:*  
- `projects` - One or more (space separated) projects to disable.

*Options:*
- `--no-dependency-checking`, `-n` - Disable dependency-checking and disable module(s) regardless. This could cause problems if there are other enabled modules that depend on this one. Use with caution.

*Examples:*  
- `bee disable webform` - Disable the Webform module.
- `bee disable --no-dependency-checking rules` - Disable the Rules module, regardless of whether or not other modules depend on it.  
- `bee disable simplify thesis bamboo` - Disable the Simplify module, Thesis theme, and Bamboo layout.  

#### `uninstall`
*Description:* Uninstall one or more modules.  
*Aliases:* `pmu` , `pm-uninstall`  
*Arguments:*
- `modules` - One or more (space separated) modules to uninstall.

*Options:*
- `--no-dependency-checking`, `-n` - Disable dependency-checking and uninstall module(s) regardless. This could cause problems if there are other installed modules that depend on this one. Use with caution.

*Examples:*
- `bee uninstall webform` - Uninstall the Webform module.
- `bee uninstall --no-dependency-checking rules` - Uninstall the Rules module, regardless of whether or not other modules depend on it.
- `bee uninstall simplify thesis bamboo` - Uninstall the Simplify module, Thesis theme, and Bamboo layout. 

### Themes
#### `theme-admin`
*Description:* Set the admin theme.  
*Aliases:* `admin-theme`  
*Arguments:*
- `theme` - The theme to set as the admin theme. It will be enabled, if not already.

*Examples:*
- `bee theme-admin basis` - Set Basis as the admin theme.

#### `theme-default`
*Description:* Set the default theme.  
*Aliases:* `default-theme`  
*Arguments:*
- `theme` - The theme to set as the default theme. It will be enabled, if not already.  

*Examples:*
- `bee theme-default bartik` - Set Bartik as the default theme.

#### `theme-debug`
*Description:* Enable or disable "Theme debug" for Backdrop.  
*Aliases:*`td`  

*Arguments:*
- `value` - (optional) A boolean value to enable (true/1) or disable (false/0) theme debug. Omitting the value will return the current theme debug status.

*Examples:*
- `bee theme-debug true` - Enable theme debug for the site (not case-sensitive).
- `bee theme-debug FALSE` - Disable theme debug for the site (not case-sensitive).
- `bee theme-debug 1` - Enable theme debug for the site.
- `bee theme-debug 0` - Disable theme debug for the site.
- `bee theme-debug` - Get the theme debug status for the site.

### Information
#### `version`
*Description:* Display the current version of Bee.

*Examples:*
- `bee version` -    Output the current version.

#### `status`
*Description:* Provides an overview of the current Backdrop installation/site.   
*Aliases:*  `st` , `info` , `core-status`  
*Options:*                                             
`--show-password`, `-p` - Show the database password.

*Examples:*
- `bee status` -    Get an overview of the Backdrop installation.
- `bee status --show-password` - Include the database password in the overview.  

#### `log`
*Description:* Show database log messages.  
*Aliases:* `ws` , `dblog` , `watchdog-show`  
*Arguments:*
- `id` - (optional) The ID of a log message to display in detail. Leave blank to list summaries of the most recent log messages.

*Options:*
- `--count=NUMBER` - The number of log messages to show. Default is 10.
- `--severity=VALUE` - Restrict list to messages of a given severity level.
- `--type=TYPE` - Restrict list to messages of a given type.

*Examples:*
- `bee log 3551` - Show details about the log message with ID '3551'.
- `bee log --count=25 --severity=error --type=php` - Show the 25 latest PHP errors.
- `bee log` - Show the latest 10 log messages.

#### `help`
*Description:* Provide help and examples for 'bee' and its commands.  
*Arguments:*
- `command` - (optional) The command to display help for. Leave blank to display help for 'bee' itself.

*Examples:*
- `bee help status` - Display help for the 'status' command.
- `bee help` - Display help for 'bee' itself.

### Core
#### `download-core`
*Description:* Download Backdrop core.  
*Aliases:* `dl-core`  
*Arguments:*
- `directory` - (optional) The directory to download and extract Backdrop into. Leave blank to use the current directory.
*Options:*
- `--github-token` - A Github Personal Access Token (Classic) that can be used to extend the GitHub API rate.
- `--hide-progress`, `-h` - Deprecated, will get removed in a future version.

*Examples:*
- `bee download-core ../backdrop` - Download Backdrop into a 'backdrop' directory in the parent folder.
                    

#### `install`
*Description:* Install Backdrop and setup a new site.  
*Aliases:* `si` , `site-install`  
*Options:*  
- `--db-name=DATABASE_NAME` - The name of the database to install into.
- `--db-user=DATABASE_USERNAME` - The username for connecting to the database.
- `--db-pass=DATABASE_PASSWORD` - The password for connecting to the database.
- `--db-host=DATABASE_HOST` - The host for the database. Defaults to 'localhost' when a database name is provided.
- `--username=USERNAME` - The username for the primary administrator account. Defaults to 'admin'.
- `--password=PASSWORD` - The password for the primary administrator account. Defaults to a randomly-generated string.
- `--email=EMAIL` - The email address for the primary administrator account. Defaults to 'admin@example.com'.
- `--site-name=NAME` - The name of the site. Defaults to 'My Backdrop Site'.
- `--site-mail=EMAIL` - The 'From' address used in automated site emails. Defaults to 'admin@example.com'.
- `--profile=PROFILE` - The machine-name of the installation profile to use. Defaults to 'standard'.
- `--langcode=LANGUAGE` - The short code of the default site language. Language files must already be present. Defaults to 'en'.
- `--db-prefix=PREFIX` - The table prefix to use for this site in the database. Defaults to no prefix.
- `--no-clean-urls`, `-n` - Disable clean URLs.
- `--auto`, `-a` - Perform an automatic (i.e. non-interactive) installation. Any options not explicitly provided to the command will use default values, except the database connection string which will always prompt when not provided.

*Examples:*
- `bee install` - Install Backdrop in interactive mode, providing information when prompted.
- `bee install --db-name=backdrop --db-user=admin --db-pass=P@ssw0rd! --auto` - Install Backdrop automatically using the provided database credentials, and default settings for everything else.
- `bee install --db-name=backdrop --db-user=admin --db-pass=P@ssw0rd! --db-host=db_server --username=Root --password=N0tS3cur3 --email=root@mydomain.com --site-name="My awesome site!"` - Install Backdrop using the given options, and be prompted for the rest.

### Update
#### `update-db`
*Description:* Show, and optionally apply, all pending database updates.  
*Aliases:* `updb` , `updbst` , `updatedb` , `updatedb-status`   
*Examples:*
- `bee update-db` - Show a list of any pending database updates. You will then be prompted to apply them. 

#### `update`
TBC see https://github.com/backdrop-contrib/bee/issues/111


### Database
#### `db-export`
*Description:* Export the database as a compressed SQL file (.sql.gz). This uses the --no-tablespaces option by default.  
*Aliases:* `dbex` , `db-dump` , `sql-export` , `sql-dump`  
*Options:*
- `--extra=--MYSQLDUMPOPTIONS` - (optional) additional `mysqldump` option(s) that should be used. Enclose multiple options in "". '--no-tablespaces' option is not used unless you add it.   

*Arguments:*
- `file` - (optional) The SQL file where the exported database will be saved. Leave blank to use the current date/time as the filename.

Note: The path is always relative to the Backdrop root so if you want to export to a folder above this, use `../` and the filename.
  
*Examples:*  
- `bee db-export db.sql` - Export the database to db.sql.gz.
- `bee db-export` - Export the database to [DATE_TIME].sql.gz.
- `bee db-export --extra=" " db.sql` - Export the database to db.sql.gz without using the '--no-tablespaces' option.
- `bee db-export --extra="--no-data --no-tablespaces" db.sql` Export the database without data, and using the '--no-tablespaces' option, to db.sql.gz.

#### `db-import`
*Description:* Import an SQL file into the current database.  
*Aliases:* `dbim` , `sql-import`  
*Arguments:*  
- `file` - The SQL file to import into the database. Can be gzipped (i.e. *.sql.gz).

Note: The path is always relative to the Backdrop root so if you want to import from a folder above this, use `../` and the filename.

*Examples:*  
- `bee db-import backup.sql` - Import backup.sql into the current database.
- `bee db-import db.sql.gz` - Extract and import db.sql into the current database.
- `bee db-import ../db/db_export.sql.gz` - Extract and import db_export.sql from folder ../db into the current database

#### `db-drop`
*Description:* Drop the current database and recreate an empty database with the same details. This could be used prior to import if the target database has more tables than the source database.  
*Aliases:* `sql-dropimport`  

*Examples:*  
- `bee db-drop` - Drop the current database and recreate an empty database with the same details.  You will then be prompted to confirm.
- `bee --yes db-drop` - Drop the current database and recreate an empty database with the same details.  You will NOT be prompted to confirm.

### Roles
#### `permissions`
*Description:* List all permissions of the modules.
*Aliases:*`pls`, `permissions-list`
*Options:*
- `--module=MODULE` - Get the permissions for this module.

*Examples:*
- `bee permissions` - Display a list of all permissions of the modules for the current site.
- `bee permissions --module=node` - Display a list of all permissions from the 'node' module for the current site.

#### `roles`
*Description:* List all roles with the permissions.
*Aliases:*`rls`, `roles-list`
*Options:*
- `--role=ROLE` - Get the permissions granted to this role.

*Examples:*
- `bee roles` - Display a list of all roles with the permissions for the current site.
- `bee roles --role=editor` - Display all the permissions for the editor role.

#### `role-create`
*Description:* Add a role.
*Aliases:*`rcrt`

*Arguments:*
- `role` - Role to add.

*Examples:*
- `bee role-create manager` - Add role 'manager'.

#### `role-delete`
*Description:* Delete a role.
*Aliases:*`rdel`

*Arguments:*
- `role` - Role to delete.

*Examples:*
- `bee role-delete manager` - Delete role 'manager'.

#### `role-add-perm`
*Description:* Grant specified permission(s) to a role.
*Aliases:*`rap`

*Arguments:*
- `permissions` - Permissions
- `role` - Role

*Examples:*
- `bee role-add-perm  'post comments' 'anonymous'` - Allow anonymous users to post comments.
- `bee role-add-perm  "'view own unpublished content' , 'view any unpublished content' , 'view revisions'" 'anonymous'` - Grant multiple permissions to the 'anonymous' role.

#### `role-remove-perm`
*Description:* Remove specified permission(s) from a role.
*Aliases:*`rrp`

*Arguments:*
- `permissions` - Permissions
- `role` - Role

*Examples:*
- `bee role-remove-perm 'access content' 'anonymous'` - Hide content from anonymous users.
- `bee role-remove-perm  "'view own unpublished content' , 'view any unpublished content' , 'view revisions'" 'anonymous'` - Remove multiple permissions from the 'anonymous' role.

### Users
#### `user-create`
*Description:* Create a user account with the specified name.  
*Aliases:* `ucrt`  
*Arguments:*
- `username` - The username of the account being created.  

*Options:*
- `--mail=EMAIL` - The email address for the new account; it must be unique and valid in this installation.  
- `--password=PASSWORD` - The password for the new account.  

*Examples:*  
- `bee user-create joe --mail=joe@example.com` - Create the 'joe' user account. A random password will be generated.
- `bee user-create joe --mail=joe@example.com --password=P@55w0rd` - Create the 'joe' user account with a defined password.
- `bee user-create joe --mail=joe@example.com --password="Correct Horse Battery Staple"` - Create the 'joe' user account with a pass phrase that has spaces.

#### `user-cancel`
*Description:* Cancel/remove a user.  
*Aliases:* `ucan`  
*Arguments:*
- `username` - The username of the account to cancel/remove.

*Examples:*  
- `bee user-cancel joe` - Cancel/remove the 'joe' user account.

#### `user-block`
*Description:* Block a user.  
*Aliases:* `ublk`  
*Arguments:*
- `username` - The username of the account to block.

*Examples:*  
- `bee user-block joe` - Block the 'joe' user account.

#### `user-unblock`
*Description:* Unblock a user.  
*Aliases:* `uublk`  
*Arguments:*
- `username` - The username of the account to unblock.

*Examples:*  
- `bee user-block joe` - Unblock the 'joe' user account.

#### `user-add-role`
*Description:* Add role to user.  
*Aliases:* `urole`, `urol`  
*Arguments:*
- `role` - Role to add.
- `username` - The username of the account to add role to.

*Examples:*  
- `bee user-add-role editor joe` - Add role 'editor' to account 'joe'.

#### `user-remove-role`
*Description:* Remove a role from a user.  
*Aliases:* `urrole`,`urrol`  
*Arguments:*
- `role` - Role to remove.
- `username` - The username of the account to remove the role from.

*Examples:*  
- `bee user-remove-role joe editor` - Remove role 'editor' from account joe.

#### `user-login`
*Description:* Display a one-time login link for a given user.  
*Aliases:* `uli`   
*Arguments:*
- `username` - (optional) The username of the user account to login as. Leave blank to login as User 1.

*Examples:*
- `bee user-login bob` - Generate and display a one-time login link for the user account named 'bob'.
- `bee user-login` - Generate and display a one-time login link for User 1.
- `bee user-login --base-url=https://www.example.com` - Generate and display a one-time login link for User 1, setting the base-url option.

#### `user-password`
*Description:* Reset the login password for a given user.  
*Aliases:* `upw` , `upwd`  
*Arguments:*
- `username` - The username of the user account to reset the password for.
- `password` - (optional) The new password to use for the given user account. Leave blank to generate a random password.

*Examples:*
- `bee user-password admin P@55w0rd` - Give the 'admin' user account an insecure password (not recommended).
- `bee user-password drop "Too hard to guess, too long to brute-force."` - Give the 'drop' user account a secure password.
- `bee user-password "Joe Bloggs"` - Give the 'Joe Bloggs' user account a random password.

#### `users`
*Description:* List all user accounts.  
*Aliases:* `uls` , `user-list`   
*Examples:*
- `bee users` - Display a list of all user accounts for the current site.

### Miscellaneous
#### `cache-clear`
*Description:* Clear a specific cache, or all Backdrop caches.  
*Aliases:* `cc`  
*Arguments:*  
- `cache` - (optional) The name of the cache to clear. Leave blank to see a list of available caches.  

*Examples:*  
- `bee cache-clear menu` - Clear the menu cache.  
- `bee cache-clear all` - Clear all caches.
- `bee cache-clear css_js` - Clear the CSS & JS cache.  
- `bee cache-clear` - Select the cache to clear from a list of available options: 
  
0. All (default)
1. Core (page, admin bar, etc.)
2. CSS & JS
3. Entity
4. Layout
5. Menu
6. Module
7. Theme
8. Token
9. Update

#### `cron`
*Description:* Run cron.  
*Examples:*
- `bee cron` - Initiate a cron run.

#### `maintenance-mode`
*Description:* Enable or disable maintenance mode for Backdrop.  
*Aliases:* `mm`  
*Arguments:*
- `value` - (optional) A boolean value to enable (true/1) or disable (false/0) maintenance mode.
Omitting the value will return the current maintenance mode status.

*Examples:*
- `bee maintenance-mode true` - Enable maintenance mode for the site (not case-sensitive).
- `bee maintenance-mode FALSE` - Disable maintenance mode for the site (not case-sensitive).
- `bee maintenance-mode 1` - Enable maintenance mode for the site.
- `bee maintenance-mode 0` - Disable maintenance mode for the site.
- `bee maintenance-mode` - Get the maintenance mode status for the site.

### State
#### `state-get`
*Description:* Get the value of a Backdrop state.  
*Aliases:* `sg`, `sget`  
*Arguments:*
- `state` - The name of the state to get.

*Examples:*
- `bee state-get maintenance_mode` - Get the value of the 'maintenance_mode' state.

#### `state-set`
*Description:* Set the value of an existing Backdrop state.  
*Aliases:* `ss`, `sset`  
*Arguments:*
- `state` - The name of the state to set.
- `value` - The value to set the state to.

*Examples:*
- `bee state-set maintenance_mode 1` - Set the value of the 'maintenance_mode' state to '1' (enabled).
- `bee state-set database_utf8mb4_active TRUE` - Set the value of the 'database_utf8mb4_active' state to 'TRUE'.

### Advanced
#### `eval`
*Description:* Evaluate (run/execute) arbitrary PHP code after bootstrapping Backdrop.  
*Aliases:* `ev` , `php-eval`   
*Arguments:*
- `code` - The PHP code to evaluate.

*Examples:*
- `bee eval '$node = node_load(1); print $node->title;'` - Loads node with nid 1 and then prints its title.
- `bee eval "node_access_rebuild();"` - Rebuild node access permissions.
- `bee eval "file_unmanaged_copy('$HOME/Pictures/image.jpg', 'public://image.jpg');"` - Copies a file whose path is determined by an environment's variable. Note the use of double quotes so the variable $HOME gets replaced by its value.  

#### `php-script`
*Description:* Execute an arbitrary PHP file after bootstrapping Backdrop.  
*Aliases:* `scr`  
*Arguments:*
- `file` - The file you wish to execute with extension and path. The path to the file should be relative to the Backdrop site root directory, or the absolute path.  

*Examples:*
- `bee php-script ../my-scripts/scratch.php` - Run scratch.php script relative to the Backdrop root.
- `bee scr /var/www/my-scripts/scratch.php` - Run scratch.php script with the absolute path.

#### `sql`
*Description:* Open an SQL command-line interface using Backdrop's database credentials.  
*Aliases:* `sqlc` , `sql-cli` , `db-cli`  
*Examples:*
- `bee sql` - Open a command-line for the current database.
- `bee sql < ../backup.sql` - Use SQL statements in a file to import data into the current database.
- `bee sql < ../scripts/sanitize.sql` - Use SQL statements in a file to complete operations with the current database.

#### `db-query`
*Description:* Execute a query using `db_query()`.  
*Aliases:* `dbq`  
*Examples:*
- `bee db-query "SELECT * FROM {users} WHERE uid = 1"` - Browse user record. Note that table prefixes are honored.
- `bee db-query "UPDATE {users} SET mail = 'me@example.com' WHERE uid = 1"` - Update a user's email address.
- `bee db-query "DELETE FROM {users} WHERE uid = 2"` - Delete a user record from the db.
- `bee db-query "SELECT name, mail FROM {users} WHERE uid > 0 LIMIT 2"` - Show the name and email for the first 2 users.