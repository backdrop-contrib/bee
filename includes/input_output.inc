<?php
/**
 * @file
 * Functions for dealing with terminal input & output.
 */

/**
 * Add a message to the `$messages` array.
 *
 * @param string $message
 *   The message to be displayed.
 * @param string $type
 *   The type of message to be displayed. One of: status, success, warning,
 *   error, log.
 *
 * @return array
 *   An array of messages to be displayed, grouped by type.
 */
function b_messages($message = FALSE, $type = FALSE) {
  static $messages = array();

  if ($message) {
    if (!isset($messages[$type])) {
      $messages[$type] = array();
    }
    $messages[$type][] = $message;
  }

  return $messages;
}

/**
 * An alias for `b_messages()`, defaulting to 'status' type messages.
 *
 * @see b_messages()
 */
function b_set_message($message = FALSE, $type = 'status') {
  b_messages($message, $type);
}

/**
 * Format and display all messages to the user.
 */
function b_print_messages() {
  global $debug_mode;
  $rows = array();

  // Get all messages.
  if (empty($messages = b_messages())) {
    return;
  }

  // Style each type of message.
  foreach ($messages as $type => $type_messages) {
    if ($type == 'log' && !$debug_mode) {
      continue;
    }

    switch ($type) {
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

      case 'status':
      default:
        $color = 'blue';
        $bold = TRUE;
        $symbol = '[o]';
    }

    foreach ($type_messages as $message) {
      $rows[] = array(
        array(
          'data' => $symbol,
          '#color' => $color,
          '#bold' => $bold,
        ),
        $message,
      );
    }
  }

  // Render messages in a table.
  b_render(array(
    '#type' => 'table',
    'rows' => $rows,
    'delimiter' => '   ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  ));
  echo "\n";
}





/**
 * Asks the user a basic yes/no question.
 *
 * @param string $msg
 *   The question to ask.
 *
 * @return bool
 *   TRUE if the user enters "y" or FALSE if "n".
 */
function b_confirm($msg) {
  global $yes_mode;

  $text = array(
    '#color' => 'red',
    'value' => $msg . " (y/N): ",
  );
  render_text($text, FALSE);

  // Automatically accept confirmations if the --yes argument was supplied.
  if ($yes_mode) {
    b_render_text("y");
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
    b_render_text($msg . " (y/N): ", FALSE);
  }
}

/**
 * Ask the user to select an item from a list.
 *
 * From a provided associative array, drush_choice will
 * display all of the questions, numbered from 1 to N,
 * and return the item the user selected. "0" is always
 * cancel; entering a blank line is also interpreted
 * as cancelling.
 *
 * @param array $options
 *   A list of questions to display to the user.  The
 *   KEYS of the array are the result codes to return to the
 *   caller; the VALUES are the messages to display on
 *   each line. Special keys of the form '-- something --' can be
 *   provided as separator between choices groups. Separator keys
 *    don't alter the numbering.
 * @param string $title
 *   The message to display to the user prompting for input.
 */
function b_choice($options, $title) {
  global $yes_mode;

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
  foreach ($keys as $num => $value) {
    $rows[] = array(
      array('data' => '[' . ($num + 1) . ']', '#color' => 'bold_blue'),
      $options[$value],
    );
  }

  // Automatically accept confirmations if the --yes argument was supplied.
  if ($yes_mode) {
    b_render_text("y");
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

    if (isset($keys[$num - 1])) {
      return $keys[$num - 1];
    }

    render_text($text, FALSE);
  }
}

/**
 * Prompt user for input.
 *
 * @param string $prompt
 *   The prompt for user input.
 *
 * @param string $default
 *   The defualt if the user just hits enter.
 */
function b_prompt($prompt, $default) {
  if (isset($default)) {
    $prompt .= " [" . $default . "]";
  }
  $prompt .= ": ";

  echo "\t\e[38;5;62m$prompt\e[0m";

  $stdin = fopen('php://stdin', 'r');

  while (($line = fgets($stdin)) !== FALSE) {
    $line = trim($line);
    if ($line === "") {
      $line = $default;
    }
    if ($line) {
      break;
    }
  }
  fclose($stdin);

  return $line;
}
