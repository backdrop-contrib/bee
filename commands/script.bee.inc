<?php
/**
 * @file
 * Command(s) for evaluating PHP code.
 */

/**
 * Implements hook_bee_command().
 */
function script_bee_command() {
  return array(
    'php-script' => array(
      'description' => bt('Execute an arbitrary PHP file after bootstrapping Backdrop.'),
      'callback' => 'script_bee_callback',
      'arguments' => array(
        'file' => bt('The file you wish to execute with extension and path. The path to the file should be relative to the Backdrop site root directory, or the absolute path.'),
      ),
      'aliases' => array('scr'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee php-script ../my-scripts/scratch.php' => bt('Run scratch.php script relative to the Backdrop root.'),
        'bee scr /var/www/my-scripts/scratch.php' => bt('Run scratch.php script with the absolute path.'),
      ),
    ),
  );
}

/**
 * Command callback: Execute php scripts after bootstrapping Backdrop.
 */
function script_bee_callback($arguments, $options) {
  if (!file_exists($arguments['file'])) {
    return;
  }

  try {
    include($arguments['file']);
  } catch (ParseError $e) {
    // This is more readable than the default error we would get from PHP.
    $err_msg = bt('!msg in: !file', array(
      '!msg' => $e->getMessage(),
      '!file' => $arguments['file'],
    ));
    bee_message($err_msg, 'error');
  }

}
