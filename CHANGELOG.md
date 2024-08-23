# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project follows the
[Backdrop CMS versioning standard](https://github.com/backdrop-ops/contrib#releases)
which is based on the major version of Backdrop CMS with a semantic version
system for each contributed module, theme and layout.

## [Unreleased]

### Added
- A new function to check whether or not an executable exists in the system.
- A new function to display messages and data at the point they happen in code.
- New commands for roles and permissions:
  - List all roles with permissions
  - List all permissions by module
  - Create or delete roles
  - Add or remove permissions from roles
- An implementation of Backdrop's telemetry function.
- A config-clear command.

### Changed
- Call the install script using the PHP_BINARY constant to avoid issues if the
execute permissions have not been added to the file.
- Symbols now defined as constants that can be used anywhere.
- Changed the help output to text rather than tables.
- Changed the version number normally present to be more meaningful and added the latest version number.
- Messages output at the conclusion of the operation are now output as text rather than tables.
- The Wiki is now edited via the `docs` folder of the main repo. Pages can be edited as part of a pull request.
- Added 'context' of 'Bee' to any translatable strings passed to `t()`.
- Moved 'theme' commands and tests from 'projects' to dedicated files.

### Fixed
- Unhandled Error if the executables called in the database commands does
not exist.
- Unhandled Warning if argument for 'cache-clear' wasn't in the list.
- Error if config-set used to set a new configuration item.
- Unhandled Warning on `user-create` with mail option.

## [1.x-1.0.2] - 2024-05-23

### Added
- Configuration and tool (Box) to build Phar files.
- Support for reporting database config storage in status command.

### Changed
- Updated Lando file to reflect Lando v3.21.0.
- Lando dev/testing recipe updated to allow to switch bee version with rebuild.

### Fixed
- GitHub Action tests failing.
- Warning if database settings in array and port not included.
- Failure to find existing core submodule dependencies.

## [1.x-1.0.1] - 2024-04-24

### Added
- PHP eval command
- User login and password reset commands
- config import/export commands
- Ability to download the dev branch of projects
- Database and SQL commands
- Documentation about extending Bee
- Support for PHP 8+
- Version command
- Maintenance mode and state commands
- Ability to download project dependencies
- PHPCS checks included in GitHub Actions
- PHP Script command
- DB Drop command
- Support command level short form options
- DB Query command
- Support for GitHub Tokens to mitigate GitHub API rate limit.

### Changed
- Improved install command
- Improved support for different versions of `wget()`
- Extended status command
- Improved display of help
- Replace `exec()`and `shell_exec()` in download command
- Additional location for custom commands
- Improve error handling
- Improve support for running tests outside Lando

### Removed
- Hide progress option within download

### Fixed
- Multiple bug fixes

## [1.x-1.0.0-beta] - 2021-03-22

### Added
- Cache commands
- Config commands
- Cron command
- DB Log commands
- Download commands
- Help commands
- Install command
- Project commands
- Status command
- Update DB command
- Tests
- Lando file to run tests locally
- Add messages and output capability
- Add multisite support
### Changed
- Changed from `b` to `bee`

[Unreleased]: https://github.com/backdrop-contrib/bee/compare/1.x-1.0.2...HEAD
[1.x-1.0.2]: https://github.com/backdrop-contrib/bee/compare/1.x-1.0.1...1.x-1.0.2
[1.x-1.0.1]: https://github.com/backdrop-contrib/bee/compare/1.x-1.0.0-beta...1.x-1.0.1
[1.x-1.0.0-beta]: https://github.com/backdrop-contrib/bee/compare/v0.0.0...1.x-1.0.0-beta
