<?php
/**
 * @file
 * Command(s) for evaluating PHP code directly.
 */

/**
 * Implements hook_b_command().
 */
function eval_b_command() {
  return array(
    'php-eval' => array(
      'description' => bt('Evaluate arbitrary PHP code after bootstrapping Backdrop.'),
      'callback' => 'php_eval_b_command',
      'arguments' => array(
        'code' => bt('The PHP code to evaluate.'),
      ),
      'aliases' => array('ev', 'eval'),
      'bootstrap' => B_BOOTSTRAP_FULL,
      'examples' => array(
        'b php-eval \'$node = node_load(1); print $node->title;\'' => 'Loads node with nid 1 and then prints its title.',
        'b php-eval "node_access_rebuild();"' => 'Rebuild node access permissions.',
        'b php-eval "file_unmanaged_copy(\'$HOME/Pictures/image.jpg\', \'public://image.jpg\');"' => 'Copies a file whose path is determined by an environment\'s variable. Note the use of double quotes so the variable $HOME gets replaced by its value.',
      ),
    ),
  );
}

/**
 * Command callback: Execute specified PHP code.
 */
function php_eval_b_command($arguments, $options) {
  try {
    eval($arguments['code']);
  }
  catch(ParseError $e) {
    // This is more readable than the default error we would get from PHP.
    $err_msg = $e->getMessage() . ' ' . bt('in:') . ' ' . $arguments['code'];
    b_message($err_msg, 'error');
  }
}
