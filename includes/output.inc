<?php


/**
 * Rudimentary replacement for Backdrop API t() function.
 *
 * @param string
 *   String to process, possibly with replacement item.
 * @param array
 *  An associative array of replacement items.
 *
 * @return
 *   The processed string.
 *
 * @see t()
 */
function bt($string, $args = array()) {
  if (function_exists('t')) {
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
 * @param $message
 *   The message to print.
 * @param $newline
 *    Add a "\n" to the end of the output.  Defaults to TRUE.
 */
function b_print($message = '', $newline = TRUE) {
  $message = (string)$message;
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
