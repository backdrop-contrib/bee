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
     'drush' => 'Use .drush.inc files instead. Drupal 7 drush commands compatibility.',
     'y' => 'Force Yes to all Yes/No questions',
     'yes' => 'Force Yes to all Yes/No questions',
     'd' => 'Debug mode',
     'debug' => 'Debug mode on',
    ),
  );
  b_get_command_args_options($arguments, $options, $command);

  $_backdrop_root = FALSE;
  if (isset($options['root'])) {
    if (file_exists($options['root'] . '/settings.php')) {
      $_backdrop_root =  $options['root'];
    }
  }
  else {
    $path = getcwd();
    if (file_exists($path . '/settings.php')) {
      $_backdrop_root = $path;
    }
  }

  $hostname = 'localhost';
  if (!is_dir($_backdrop_root . '/core')) {
    // Could be a multisite install.
    $parent_dir = dirname($_backdrop_root);
    if (file_exists($parent_dir . '/sites.php')) {
      require_once($parent_dir . '/sites.php');
      $base = basename($_backdrop_root);
      $key = array_search($base, $sites);
      if ($key !== FALSE) {
        $hostname = $key;
        $_backdrop_root = dirname($parent_dir);
      }
    }
  }
  
  if ($_backdrop_root) {
    define('BACKDROP_ROOT', $_backdrop_root);
    chdir(BACKDROP_ROOT);
    require_once BACKDROP_ROOT . '/core/includes/bootstrap.inc';

    if (function_exists('backdrop_bootstrap_is_installed')) {
      $_SERVER['HTTP_HOST'] = $hostname;
      backdrop_settings_initialize();
      if (backdrop_bootstrap_is_installed()) {
        b_backdrop_installed(TRUE);
      }
      else {
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
