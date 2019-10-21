<?php
/**
 * @file
 * The drush API implementation and helpers.
 */

/**
 * @name Error status definitions
 * @{
 * Error code definitions for interpreting the current error status.
 *
 * @see drush_set_error(), drush_get_error(), drush_get_error_log(),
 *      drush_cmp_error()
 */
// The command completed successfully.
define('DRUSH_SUCCESS', 0);

// The command could not be completed because the framework has specified errors
// that have occured.
define('DRUSH_FRAMEWORK_ERROR', 1);

// The command was aborted because the user chose to cancel it at some prompt.
// This exit code is arbitrarily the same as EX_TEMPFAIL in sysexits.h, although
// note that shell error codes are distinct from C exit codes, so this alignment
// not meaningful.
define('DRUSH_EXITCODE_USER_ABORT', 75);

// The command that was executed resulted in an application error.
// The most commom causes for this is invalid PHP or a broken SSH pipe when
// using drush_backend_invoke in a distributed manner.
define('DRUSH_APPLICATION_ERROR', 255);

/**
 * Convert a csv string, or an array of csv strings, into an array of items.
 *
 * @param string|array $args
 *   A simple csv string; e.g. 'a,b,c'
 *   or a simple list of items; e.g. array('a','b','c')
 *   or some combination; e.g. array('a,b','c') or array('a,','b,','c,')
 *
 * @return array
 *   A simple list of items (e.g. array('a','b','c')
 */
function _convert_csv_to_array($args) {
  // Step 1: implode(',',$args) converts from, say, array('a,','b,','c,') to
  // 'a,,b,,c,'.
  // Step 2: explode(',', ...) converts to array('a','','b','','c','').
  // Step 3: array_filter(...) removes the empty items.
  // Step 4: array_map(...) trims extra whitespace from each item (handles csv
  // strings with extra whitespace, e.g. 'a, b, c').
  return array_map('trim', array_filter(explode(',', is_array($args) ? implode(',', $args) : $args)));
}


/**
 * Set an error code for the error handling system.
 *
 * @param \Drupal\Component\Render\MarkupInterface|string $error
 *   A text string identifying the type of error.
 * @param null|string $message
 *   Optional. Error message to be logged. If no message is specified,
 *   hook_drush_help will be consulted, using a key of 'error:MY_ERROR_STRING'.
 * @param null|string $output_label
 *   Optional. Label to prepend to the error message.
 *
 * @return bool
 *   Always returns FALSE, to allow returning false in the calling functions,
 *   such as <code>return drush_set_error('DRUSH_FRAMEWORK_ERROR')</code>.
 */
function drush_set_error($error, $message = NULL, $output_label = "") {
  if (is_numeric($error)) {
    $error = 'DRUSH_FRAMEWORK_ERROR';
  }
  elseif (!is_string($error)) {
    $error = "$error";
  }
  $message = ($message) ? $message : 'error:' . $error;
  if (is_array($message)) {
    $message = implode("\n", $message);
    foreach ($message as $line) {
      b_set_message($line, 'error');
    }
  }
  else {
    b_set_message($message, 'error');
  }
}

/**
 * Add a log message to the log history.
 *
 * This function calls the callback stored in the 'DRUSH_LOG_CALLBACK' context
 * with the resulting entry at the end of execution.
 *
 * This allows you to replace it with custom logging implementations if needed,
 * such as logging to a file or logging to a database (drupal or otherwise).
 *
 * The default callback is the Drush\Log\Logger class with prints the messages
 * to the shell.
 *
 * @param string $message
 *   String containing the message to be logged.
 * @param string $type
 *   The type of message to be logged. Common types are 'warning', 'error',
 *   'success' and 'notice'. A type of 'ok' or 'success' can also be supplied to
 *   flag something that worked. If you want your log messages to print to
 *   screen without the user entering a -v or --verbose flag, use type 'ok' or
 *   'notice', this prints log messages out to STDERR, which prints to screen
 *   (unless you have redirected it). All other types of messages will be
 *   assumed to be info.
 */
function drush_log($message, $type = 'ok', $error = NULL) {
  $status = 'log';
  if ($error) {
    $status = 'error';
  }

  switch ($type) {
    case 'ok':
      $status = 'status';
      break;

    case 'success':
      $status = 'success';
      break;
  }
  b_set_message($message, $status);
  /*$entry = array(
  'type' => $type,
  'message' => $message,
  'timestamp' => microtime(TRUE),
  'memory' => memory_get_usage(),
  );
  $entry['error'] = $error;
  return _drush_log($entry);*/
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
 *   A list of questions to display to the user. The KEYS of the array are the
 *   result codes to return to the caller; the VALUES are the messages to
 *   display on each line. Special keys of the form '-- something --' can be
 *   provided as separator between choices groups. Separator keys don't alter
 *   the numbering.
 * @param string $prompt
 *   The message to display to the user prompting for input.
 * @param string $label
 *   Controls the display of each line.  Defaults to '!value', which displays
 *   the value of each item in the $options array to the user.  Use '!key' to
 *   display the key instead. In some instances, it may be useful to display
 *   both the key and the value; for example, if the key is a user id and the
 *   value is the user name, use '!value (uid=!key)'.
 */
function drush_choice($options, $prompt = 'Enter a number.', $label = '!value', $widths = array()) {
  drush_print(dt($prompt));
  // Preflight so that all rows will be padded out to the same number of
  // columns.
  $array_pad = 0;
  foreach ($options as $key => $option) {
    if (is_array($option) && (count($option) > $array_pad)) {
      $array_pad = count($option);
    }
  }
  $rows[] = array_pad(array('[0]', ':', 'Cancel'), $array_pad + 2, '');
  $selection_number = 0;
  foreach ($options as $key => $option) {
    if ((substr($key, 0, 3) == '-- ') && (substr($key, -3) == ' --')) {
      $rows[] = array_pad(array('', '', $option), $array_pad + 2, '');
    }
    else {
      $selection_number++;
      $row = array("[$selection_number]", ':');
      if (is_array($option)) {
        $row = array_merge($row, $option);
      }
      else {
        $row[] = dt($label, array(
          '!number' => $selection_number,
          '!key' => $key,
          '!value' => $option,
        ));
      }
      $rows[] = $row;
      $selection_list[$selection_number] = $key;
    }
  }
  drush_print_table($rows, FALSE, $widths);
  flush();
  ob_flush();
  if (($choice = drush_get_option('choice', FALSE)) !== FALSE) {
    // First check to see if $choice is one of the symbolic options.
    if (array_key_exists($choice, $options)) {
      return $choice;
    }
    // Next handle numeric selections.
    elseif (array_key_exists($choice, $selection_list)) {
      return $selection_list[$choice];
    }
    return FALSE;
  }
  // If the user specified --no, then cancel; also avoid getting hung up waiting
  // for user input in --pipe and backend modes. If none of these apply, then
  // wait, for user input and return the selected result.
  while ($line = trim(fgets(STDIN))) {
    if (array_key_exists($line, $selection_list)) {
      return $selection_list[$line];
    }
  }
  // We will allow --yes to confirm input if there is only one choice;
  // otherwise, --yes will cancel to avoid ambiguity.
  if (b_yes_mode()  && (count($options) == 1)) {
    return $selection_list[1];
  }
  return FALSE;
}
