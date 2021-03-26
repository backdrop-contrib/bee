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
  );
}

/**
 * Command callback: Evaluate (run/execute) arbitrary PHP code after
 * bootstrapping Backdrop.
 */
function eval_bee_callback($arguments, $options) {
  try {
    eval($arguments['code'] . ';');
  }
  catch(ParseError $e) {
    // This is more readable than the default error we would get from PHP.
    $err_msg = bt('!msg in: !code', array(
      '!msg' => $e->getMessage(),
      '!code' => $arguments['code'],
    ));
    bee_message($err_msg, 'error');
  }
}
