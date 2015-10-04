b
----

`b` clears cache for Backdrop CMS.
  * that's it.

---
INSTALATION

* clone this repo somewhere on your computer (I use my home directory)
  * `git clone git@github.com:serundeputy/b.git`
* make an alias to `b`
  * open your `.bash_profile` file
  * add a line (at the end is fine) like this `alias b='php /path/to/b/b.php'`
* Make b.php executable
  * `chmod a+x b.php`
* Configure `b.php`
  * Change the `$settings_path` variable to reflect the path of your backdrop installation.

---
USING `b`
* Type `b` from inside your Backdrop CMS installation will bring up the help items.
* Type `b cc all` to clear all cache
* Type `b cc` brings up the cache clear menu
  * currently only `all` and `cancel` work
  * TODO: get the other cache-clears mapped and working
  
