<?php

set_error_handler('b_errorHandler');

require_once('includes/common.inc');
require_once('includes/command.inc');
require_once('includes/render.inc');
require_once('includes/output.inc');
require_once('includes/filesystem.inc');

//Global variables.
$elements = array();

b_init();

if (drush_mode()) {
  require_once('includes/drush_wrapper.inc');
  drush_process_command();
}
else {
  b_process_command();
}

b_print_messages();
b_render($elements);

/**
 * @param $errno
 *  todo: is this used?
 * @param string $message
 *  Message to output to the user.
 * @param string $filename
 *  The file that the error came from.
 * @param string $line
 *  The line number the error came from.
 * @param $context
 *  todo: is this used?
 */
function b_errorHandler($errno, $message, $filename, $line, $context) {
  echo $message."\n";
  echo "\t". $filename . ":" . $line ."\n";
}
exit();

/**
 * Initialize Backdrop Console.
 */
function b_init() {
  $arguments = array();
  $options = array();
  $command = array(
   'options' => array(
     'root' => 'Backdrop root folder',
     'url' => 'Backdrop site URL (as defined in sites.php)',
     'drush' => 'Use .drush.inc files instead. Drupal 7 drush commands compatibility.',
     'y' => 'Force Yes to all Yes/No questions',
     'yes' => 'Force Yes to all Yes/No questions',
     'd' => 'Debug mode',
     'debug' => 'Debug mode on',
    ),
  );
  b_get_command_args_options($arguments, $options, $command);

  b_init_blobals();

  // Get root directory.
  if (isset($options['root'])) {
    $_backdrop_root = b_find_root($options['root'], FALSE);
  }
  else {
    $_backdrop_root = b_find_root(getcwd());
  }

  if ($_backdrop_root) {
    // Get site directory.
    if (isset($options['url'])) {
      $_backdrop_site = b_find_site_by_url($options['url'], $_backdrop_root);
    }
    else {
      $_backdrop_site = b_find_site_by_path(getcwd(), $_backdrop_root);
    }

    chdir($_backdrop_root);
    $full_path = getcwd();
    define('BACKDROP_ROOT', $full_path);
    $_SERVER['HTTP_HOST'] = substr($full_path, strrpos($full_path, '/') + 1);
    if ($_backdrop_site) {
      define('BACKDROP_SITE', $_backdrop_site);
      $_SERVER['HTTP_HOST'] = $_backdrop_site;
    }
    require_once 'core/includes/bootstrap.inc';
    if(function_exists('backdrop_bootstrap_is_installed')){
      backdrop_settings_initialize();
      if(backdrop_bootstrap_is_installed()){
        b_backdrop_installed(TRUE);
      }
      else{
        b_backdrop_installed(FALSE);
        b_set_message(bt('BackdropCMS is not installed yet.'), 'warning');
      }
    }
  }

  if (isset($options['drush'])) {
    drush_mode(TRUE);
    b_set_message('Drush mode on');
  }

  if (isset($options['y']) or isset($options['yes'])) {
    b_yes_mode(TRUE);
    b_set_message('Yes mode on');
  }
  if (isset($options['d']) or isset($options['debug'])) {
    b_is_debug(TRUE);
    b_set_message('Debug mode on');
  }

}

function b_init_blobals() {
  $host = 'localhost';
  $path = '';

  $_SERVER['HTTP_HOST'] = $host;
  $_SERVER['REMOTE_ADDR'] = '127.0.0.1';
  $_SERVER['SERVER_ADDR'] = '127.0.0.1';
  $_SERVER['SERVER_SOFTWARE'] = NULL;
  $_SERVER['SERVER_NAME'] = 'localhost';
  $_SERVER['REQUEST_URI'] = $path . '/';
  $_SERVER['REQUEST_METHOD'] = 'GET';
  $_SERVER['SCRIPT_NAME'] = $path . '/index.php';
  $_SERVER['PHP_SELF'] = $path . '/index.php';
  $_SERVER['HTTP_USER_AGENT'] = 'Backdrop command line';

  if (!empty($_SERVER['HTTPS']) && $_SERVER['HTTPS'] == 'on') {
    // Ensure that any and all environment variables are changed to https://.
    foreach ($_SERVER as $key => $value) {
      $_SERVER[$key] = str_replace('http://', 'https://', $_SERVER[$key]);
    }
  }
}
