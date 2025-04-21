If your module has Drush integration via a `MODULE.drush.inc` file, you can use that as a basis for creating a `MODULE.bee.inc` integration. See [Extending Bee](https://github.com/backdrop-contrib/bee/wiki/Extending-Bee) for some background. 

## Mapping Drush functions to Bee

Not every Drush internal function has a Bee equivalent (though the maintainer is open to adding more functions if it's helpful), and the command definition might be slightly different too.

* `drush_print_table()` becomes `bee_render_table()`
* `drush_print()` becomes `bee_message($message, 'info')`
* `drush_print_help()` becomes `bee_message($message, 'info')`
* `drush_set_error()` becomes `bee_message($message, 'error')`
* `drush_get_option('name')` becomes `$options['name']` -- but only within the callback
* `drush_get_arguments('name')` becomes `$arguments['name']` -- but only within the callback

## Examples

Examples of modules that contain integrations for both Drush and Bee:
* The [S3 File System module](https://github.com/backdrop-contrib/s3fs)
* The [Elysia Cron module](https://github.com/backdrop-contrib/elysia_cron)
