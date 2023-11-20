<?php
/**
 * @file
 * Error handling for bee.
 */

/**
 * Custom error handler for `bee`.
 *
 * @param int $error_level
 *   The level of the error.
 * @param string $message
 *   Error message to output to the user.
 * @param string $filename
 *   The file that the error came from.
 * @param int $line
 *   The line number the error came from.
 * @param array $context
 *   An array of all variables from where the error was triggered.
 *
 * @see https://www.php.net/manual/en/function.set-error-handler.php
 * @see _backdrop_error_handler_real()
 */
function _bee_error_handler_real($error_level, $message, $filename, $line, array $context = NULL) {

  if ($error_level & error_reporting()) {
    $types = backdrop_error_levels();
    list($severity_msg, $severity_level) = $types[$error_level];
    $caller = _bee_get_last_caller(debug_backtrace());

    if (empty($caller)) {
      $caller = array(
        '!function' => bt('<Unknown function>'),
        '!file' => bt('<Unknown file>'),
        '!line' => bt('<Unknown line>'),
      );
    }

    // We treat recoverable errors as fatal.
    _bee_log_error(array(
      '!type' => isset($types[$error_level]) ? $severity_msg : bt('Unknown error'),
      '!message' => $message,
      '!function' => $caller['function'],
      '!file' => $caller['file'],
      '!line' => $caller['line'],
      'severity_level' => $severity_level,
    ), $error_level == E_RECOVERABLE_ERROR);
  }
}

/**
 * Logs a PHP error or exception to the console.
 *
 * @param $error
 *   An array with the following keys: !type, !message, !function, !file, !line
 *   and severity_level. All the parameters are plain-text.
 * @param $fatal
 *   TRUE if the error is fatal.
 *
 * @see _backdrop_log_error()
 */
function _bee_log_error($error, $fatal = FALSE) {

  // $severity_level is one of the WATCHDOG_* constants defined in
  // backdrop_error_levels. Map this to a bee_message type.
  switch ($error['severity_level']) {
    case WATCHDOG_ERROR:
      $msg_type = 'error';
      break;

    case WATCHDOG_WARNING:
      $msg_type = 'warning';
      break;

    case WATCHDOG_NOTICE:
      $msg_type = 'info';
      break;

    case WATCHDOG_DEBUG:
      $msg_type = 'log';
      break;
  }

  bee_message(bt("!type: !message\nin !function (line !line of !file).", $error), $msg_type);
  if ($fatal) {
    exit();
  }
}

/**
 * Gets the last caller from a backtrace.
 *
 * @param $backtrace
 *   A standard PHP backtrace.
 *
 * @return
 *   An associative array with keys 'file', 'line' and 'function'.
 *
 * @see _backdrop_get_last_caller().
 */
function _bee_get_last_caller($backtrace) {
  // Errors that occur inside PHP internal functions do not generate
  // information about file and line. Ignore disallowed functions.
  $denylist = array('debug', 'bee_error_handler', '_bee_exception_handler');
  while (($backtrace && !isset($backtrace[0]['line'])) || (isset($backtrace[1]['function']) && in_array($backtrace[1]['function'], $denylist))) {
    array_shift($backtrace);
  }

  // The first trace is the call itself.
  // It gives us the line and the file of the last call.
  $call = $backtrace[0];

  // The second call give us the function where the call originated.
  if (isset($backtrace[1])) {
    if (isset($backtrace[1]['class'])) {
      $call['function'] = $backtrace[1]['class'] . $backtrace[1]['type'] . $backtrace[1]['function'] . '()';
    }
    else {
      $call['function'] = $backtrace[1]['function'] . '()';
    }
  }
  else {
    $call['function'] = 'main()';
  }
  return $call;
}
