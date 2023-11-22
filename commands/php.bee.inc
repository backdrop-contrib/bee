<?php
/**
 * @file
 * Command(s) for evaluating PHP code.
 */

/**
 * Implements hook_bee_command().
 */
function php_bee_command() {
  return array(
    'eval' => array(
      'description' => bt('Evaluate (run/execute) arbitrary PHP code after bootstrapping Backdrop.'),
      'callback' => 'eval_bee_callback',
      'group' => 'advanced',
      'arguments' => array(
        'code' => bt('The PHP code to evaluate.'),
      ),
      'aliases' => array('ev', 'php-eval'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee eval \'$node = node_load(1); print $node->title;\'' => bt('Loads node with nid 1 and then prints its title.'),
        'bee eval "node_access_rebuild();"' => bt('Rebuild node access permissions.'),
        'bee eval "file_unmanaged_copy(\'$HOME/Pictures/image.jpg\', \'public://image.jpg\');"' => bt('Copies a file whose path is determined by an environment\'s variable. Note the use of double quotes so the variable $HOME gets replaced by its value.'),
      ),
    ),
    'php-script' => array(
      'description' => bt('Execute an arbitrary PHP file after bootstrapping Backdrop.'),
      'callback' => 'script_bee_callback',
      'group' => 'advanced',
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
 * Command callback: Evaluate (run/execute) arbitrary PHP code after
 * bootstrapping Backdrop.
 */
function eval_bee_callback($arguments, $options) {
  try {
    // phpcs:ignore Squiz.PHP.Eval -- integral part of the command
    eval($arguments['code'] . ';');
  }
  catch (ParseError $e) {
    // This is more readable than the default error we would get from PHP.
    $err_msg = bt('!msg in: !code', array(
      '!msg' => $e->getMessage(),
      '!code' => $arguments['code'],
    ));
    bee_message($err_msg, 'error');
  }
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
    $err_msg = bt('!msg in: !file', array(
      '!msg' => $e->getMessage(),
      '!file' => $arguments['file'],
    ));
    bee_message($err_msg, 'error');
  }
}
