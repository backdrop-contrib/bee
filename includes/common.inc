<?php
/*
 * @file
 * Common functions.
 */

/**
 * First bootstrap phase: initialize configuration.
 */
define('B_BOOTSTRAP_CONFIGURATION', 0);

/**
 * Second bootstrap phase: try to serve a cached page.
 */
define('B_BOOTSTRAP_PAGE_CACHE', 1);

/**
 * Third bootstrap phase: initialize database layer.
 */
define('B_BOOTSTRAP_DATABASE', 2);

/**
 * Fourth bootstrap phase: initialize the lock system.
 */
define('B_BOOTSTRAP_LOCK', 3);

/**
 * Fifth bootstrap phase: initialize the variable system.
 */
define('B_BOOTSTRAP_VARIABLES', 4);

/**
 * Sixth bootstrap phase: initialize session handling.
 */
define('B_BOOTSTRAP_SESSION', 5);

/**
 * Seventh bootstrap phase: set up the page header.
 */
define('B_BOOTSTRAP_PAGE_HEADER', 6);

/**
 * Eighth bootstrap phase: find out language of the page.
 */
define('B_BOOTSTRAP_LANGUAGE', 7);

/**
 * Final bootstrap phase: Backdrop is fully loaded; validate and fix input data.
 */
define('B_BOOTSTRAP_FULL', 8);

function b_print_error($text) {
  b_render(array(
    '#type' => 'text',
    '#color' => 'red',
    'value' => $text,
  )
  );
}

function b_messages($message = FALSE, $type = FALSE) {
  static $messages = array();
  if($message) {
    if(!isset($messages[$type])){
      $messages[$type] = array();
    }
    $messages[$type][] = $message;
  }
  return $messages;
}

function b_set_message($message = FALSE, $type = 'status') {
  b_messages($message, $type);
}

function b_print_messages() {
  $messages = b_messages();

  if(empty($messages)){
    return;
  }
  $rows = array();
  foreach($messages as $type => $sub_messages) {
    if($type == 'log' && !b_is_debug()) {
      continue;
    }
    switch($type) {
      case 'status':
        $color = 'bold_blue';
        $symbol = '[o]';
        break;
      case 'success':
        $color = 'green';
        $symbol = '[v]';
        break;
      case 'warning':
        $color = 'yellow';
        $symbol = '[!]';
        break;
      case 'error':
        $color = 'red';
        $symbol = '[x]';
        break;
      case 'log':
        $color = 'red';
        $symbol = ' - ';
        break;
      default:
        $color = 'red';
        $symbol = '[x]';
        break;
    };
    foreach($sub_messages as $message) {
      $rows[] = array(
        array('data' => $symbol, '#color' => $color),
        $message,
      );
    }
  }
  $output[] = array(
    '#type' => 'table',
    'rows' => $rows,
    'delimiter' => '   ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  );

  b_render($output);
  echo "\n";

}

/**
 * Set drush compatible mode and return status.
 */
function drush_mode($set = NULL) {
  static $mode;

  if($set != NULL) {
    $mode = $set;
  }
  return $mode;
}

/**
 * Set Yes to all mode and return status.
 */
function b_yes_mode($set = NULL) {
  static $mode;

  if($set != NULL) {
    $mode = $set;
  }
  return $mode;
}

/**
 * If -d specified.
 */
function b_is_debug($set = NULL) {
  static $mode;

  if($set != NULL) {
    $mode = $set;
  }
  return $mode;
}

/**
 * If backdrop installed, it will return TRUE.
 */
function b_backdrop_installed($set = NULL) {
  static $mode;

  if($set != NULL) {
    $mode = $set;
  }
  return $mode;
}

/**
 * Asks the user a basic yes/no question.
 *
 * @param string $msg
 *   The question to ask.
 * @param int $indent
 *   The number of spaces to indent the message.
 *
 * @return bool
 *   TRUE if the user enters "y" or FALSE if "n".
 */
function b_confirm($msg) {
  $text = array(
    '#color' => 'red',
    'value' => $msg . " (y/N): ",
  );
  render_text($text, FALSE);

  // Automatically accept confirmations if the --yes argument was supplied.
  if (b_yes_mode()) {
    b_print("y");
    return TRUE;
  }

  // See http://drupal.org/node/499758 before changing this.
  $stdin = fopen("php://stdin", "r");

  while ($line = fgets($stdin)) {
    $line = trim($line);
    if (strtolower($line) == 'y') {
      return TRUE;
    }
    if ($line == "") {
      return FALSE;
    }
    if ($line == 'n') {
      return FALSE;
    }
    b_print_prompt($msg . " (y/N): ");
  }
}

/**
 * Ask the user to select an item from a list.
 * From a provided associative array, drush_choice will
 * display all of the questions, numbered from 1 to N,
 * and return the item the user selected. "0" is always
 * cancel; entering a blank line is also interpreted
 * as cancelling.
 *
 * @param $options
 *   A list of questions to display to the user.  The
 *   KEYS of the array are the result codes to return to the
 *   caller; the VALUES are the messages to display on
 *   each line. Special keys of the form '-- something --' can be
 *   provided as separator between choices groups. Separator keys
 *    don't alter the numbering.
 * @param $title
 *   The message to display to the user prompting for input.
 */
function b_choice($options, $title) {
  $text = array(
    '#color' => 'blue',
    'value' => $title . ": ",
  );
  $keys = array_keys($options);

  $rows = array();
  // Default Cancel option.
  $rows[] = array(
    array('data' => '[0]', '#color' => 'red'),
    bt('Cancel'),
  );
  foreach($keys as $num => $value){
    $rows[] = array(
      array('data' => '[' . ($num + 1) . ']', '#color' => 'bold_blue'),
      $options[$value],
    );
  }

  // Automatically accept confirmations if the --yes argument was supplied.
  if (b_yes_mode()) {
    b_print("y");
    return $keys[0];
  }

  b_render(array(
    '#type' => 'table',
    'rows' => $rows,
    'delimiter' => '  :  ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  ));

  render_text("");
  render_text($text, FALSE);

  $stdin = fopen("php://stdin", "r");

  while ($line = fgets($stdin)) {
    $line = trim($line);
    $num = (int) $line;

    if ($num == "") {
      return FALSE;
    }
    if ($num == 'n') {
      return FALSE;
    }
    if ($num == 0) {
      return FALSE;
    }

    if(isset($keys[$num - 1])){
      return $keys[$num - 1];
    }

    render_text($text, FALSE);
  }
}
