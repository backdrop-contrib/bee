<?php

set_error_handler('b_errorHandler');

require_once('includes/common.inc');
require_once('includes/command.inc');
require_once('includes/render.inc');
require_once('includes/filesystem.inc');

b_init();
b_process_command();


function b_errorHandler($errno, $message, $filename, $line, $context){
  echo $message."\n";
  echo "\t". $filename . ":" . $line ."\n";
}
exit();


function b_init() {
  $arguments = array();
  $options = array();
  $command = array (
   'options' => array(
     'root' => 'Backdrop root folder'
    ),
  );
  b_get_command_args_options($arguments, $options, $command);
  if(isset($options['root'])) {
    if(file_exists($options['root'] . '/settings.php')) {
      define('BACKDROP_ROOT', $options['root']);
      chdir(BACKDROP_ROOT);
    }
  }
  else{
    $path = getcwd();
    if(file_exists($path . '/settings.php')) {
      define('BACKDROP_ROOT', $path);
    }
  }
}
