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
  * reload your bash profile: `source ~/.bash_profile`
* Make b.php executable
  * `chmod a+x b.php`


---
USING `b`
* Type `b` from inside your Backdrop CMS installation will bring up the help items.
* Type `b cc all` to clear all cache
* Type `b cc` brings up the cache clear menu
  * currently only `all` and `cancel` work
  * TODO: get the other cache-clears mapped and working
  
* v0.0.1 adds `dl` support
  * Download Backdrop core and modules
  * `b dl backdrop` download Backdrop CMS core.
  * `b dl redirect` download the redirect module to `./modules/` or `./contrib/modules`
  * `b dl redirect webform` download redirect and webform
