<?php
/**
 * @file
 * Functions for dealing with terminal output.
 */

/**
 * Rudimentary replacement for Backdrop API t() function.
 *
 * @param string $string
 *   String to process, possibly with replacement item.
 * @param array $args
 *   An associative array of replacement items.
 *
 * @return string
 *   The processed string.
 *
 * @see t()
 */
function bt($string, $args = array()) {
  if (function_exists('t')) {
    // @codingStandardsIgnoreLine -- Need to send variable here.
    return t($string, $args);
  }
  else {
    if (!empty($args)) {
      return strtr($string, $args);
    }
    else {
      return $string;
    }
  }
}

/**
 * Prints a message.
 *
 * @param string $message
 *   The message to print.
 * @param bool $newline
 *   Add a "\n" to the end of the output.  Defaults to TRUE.
 */
function b_print($message = '', $newline = TRUE) {
  $message = (string) $message;
  if ($newline) {
    $message .= "\n";
  }
  print $message;
}

/**
 * Print a prompt -- that is, a message with no trailing newline.
 */
function b_print_prompt($message) {
  b_print($message, FALSE);
}

/**
 * Format and print messages to the user.
 */
function b_print_messages() {
  $messages = b_messages();

  if (empty($messages)) {
    return;
  }
  $rows = array();
  foreach ($messages as $type => $sub_messages) {
    if ($type == 'log' && !b_is_debug()) {
      continue;
    }
    switch ($type) {
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
    }
    foreach ($sub_messages as $message) {
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
