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

### Template - use the module name
#### Functions
- What the bee commands do or should do
-

#### Details
- Link to request in module queue or link to module
- Do drush commands already exist?
  - If yes, link to file with drush commands in.
- Is anyone working on it?
- Your name (optional) and github handle