## Before Release

### Version Number

```php
/**
 * Define constant for Bee's current version.
 *
 * This will normally be '1.x-1.x' but will be changed briefly when a release
 * is generated.
 */
// Current version of Bee.
define('BEE_VERSION', '1.x-1.x');
```

- In `includes/globals.inc`, update the constant `BEE_VERSION` to the new version number (e.g. '1.x-1.0.2').

### Changelog

The Changelog is based on https://keepachangelog.com/  

#### Potential headings
```
### Added
- for new features.
### Changed
- for changes in existing functionality.
### Deprecated
- for soon-to-be removed features.
### Removed
- for now removed features.
### Fixed 
- for any bug fixes.
### Security
- in case of vulnerabilities.
```
#### Checklist
- Create a new Unreleased section and update the compare link to compare `HEAD` with the new release tag.
- Change the current Unreleased section to be the release tag.
- Create a new compare link to compare the previous release tag with this release tag.
- Compare links are structured: `https://github.com/backdrop-contrib/bee/compare/[old_tag]...[new_tag]`
- Titles of each section can be made into links with the link at the bottom. The list of links at the bottom will not be displayed if rendered in GitHub or in preview mode of an IDE. Follow the existing examples.
- Ensure all notable changes have been recorded under the correct headings.

### Wiki
Check all new commands and changes to existing commands are reflected in the Wiki.

### Commit
- Add a commit with the name 'Prepare for release 1.x-1.0.2'

## Release
- Set the tag to be the new version number.
- Set the title of the release to be the new version number.
- Prepare release notes using the generated draft release notes and the Changelog headings.
- Release

## After Release
- Delete `PACKAGING_ERRORS.txt` from the release files
- In `includes/globals.inc`, update the constant `BEE_VERSION` to '1.x-1.x'.
- Add a commit with the name 'Reset version after release 1.x-1.0.2'
- `git pull` to get latest version of `1.x-1.x` branch which will also update tags
- Switch to the latest tag `checkout 1.x-1.0.2`
- Compile the Phar file - `lando box compile`
- Edit the release and upload the `bee.phar` file that has been generated.