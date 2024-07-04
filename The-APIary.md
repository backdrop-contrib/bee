## Introduction
As documented in [Extending Bee](https://github.com/backdrop-contrib/bee/wiki/Extending-Bee) it is possible to extend Bee with both custom commands and hooks within modules.  The purpose of this page is to allow users to share custom `bee` commands that you think others would benefit from and to document any modules that have, or should have, `bee` commands. Please copy and use the templates to add to this page.

## Custom Commands

### Template - Give it a name
Your name (optional) and github handle  
The purpose of this command is to ...

<details>
<summary>Code</summary>

```php
<?php
  function custom_bee_command() {
    // your code
  }

  function custom_bee_callback() {
    // your code
  }
```

</details>

## Modules
### Memcache
#### Functions
- Retrieve statistics from memcache.
- Flush all objects from a bin.

#### Details
[Memcache](https://github.com/backdrop-contrib/memcache)

### Read Only Mode
Read Only Mode allows site administrators and developers to lock down or
freeze a production server so that maintenance or large deployments can occur
without taking the site offline.

#### Functions
- Enable: `bee readonlymode TRUE`
- Disable: `bee readonlymode FALSE`
- Check the status: `bee readonlymode`

It supports the aliases `rom` and `ro` and setting the value is
case-insensitive as well as supporting `0` and `1`.

See the [section in the README file](https://github.com/backdrop-contrib/readonlymode?tab=readme-ov-file#bee-support) for more details

#### Details
- [Read Only Mode](https://github.com/backdrop-contrib/readonlymode)

### Rules
#### Functions
- What the bee commands do or should do TBC

#### Details
- [Rules](https://github.com/backdrop-contrib/rules)

### Backup Migrate
#### Functions
* `bam-backup`: Backup a specified source associated with a Backdrop CMS website.
* `bam-destinations`: Get a list of available destinations.
* `bam-profiles`: Get a list of available settings profiles.
* `bam-restore`: Restore a saved backup to a specified source.
* `bam-saved`: Get a list of previously created backup files.
* `bam-sources`: Get a list of available sources.

#### Details
- [Backup Migrate](https://github.com/backdrop-contrib/backup_migrate)

### S3 File System
#### Functions
- What the bee commands do or should do TBC

#### Details
- [s3fs](https://github.com/backdrop-contrib/s3fs)

### Tweet Feed
#### Functions
- What the bee commands do or should do TBC

#### Details
- [Tweet Feed](https://github.com/backdrop-contrib/tweet_feed)

### Node Access Rebuild Progressive
#### Functions
- `bee node-access-rebuild-progressive`

or for short,

- `bee narp`

#### Details
- [Node Access Rebuild Progressive](https://github.com/backdrop-contrib/node_access_rebuild_progressive)


### Template
#### Functions
- What the bee commands do or should do

#### Details
- Link to request in module queue or link to module
- Do drush commands already exist?
  - If yes, link to file with drush commands in.
- Is anyone working on it?
- Your name (optional) and github handle