<?php
/**
 * @file
 * Error handling for bee.
 */

/**
 * @defgroup bee_logging_severity_levels Logging severity levels
 * @{
 * Log message severity for when Backdrop is not yet available. Equivalents of
 * WATCHDOG_*.
 *
 * @see watchdog_severity_levels()
 */

/**
 * Log message severity -- Emergency: system is unusable.
 */
define('BEE_WATCHDOG_EMERGENCY', 0);

/**
 * Log message severity -- Alert: action must be taken immediately.
 */
define('BEE_WATCHDOG_ALERT', 1);

/**
 * Log message severity -- Critical conditions.
 */
define('BEE_WATCHDOG_CRITICAL', 2);

/**
 * Log message severity -- Error conditions.
 */
define('BEE_WATCHDOG_ERROR', 3);

/**
 * Log message severity -- Warning conditions.
 */
define('BEE_WATCHDOG_WARNING', 4);

/**
 * Log message severity -- Notice: normal but significant conditions.
 */
define('BEE_WATCHDOG_NOTICE', 5);

/**
 * Log message severity -- Informational messages.
 */
define('BEE_WATCHDOG_INFO', 6);

/**
 * Log message severity -- Debug-level messages.
 */
define('BEE_WATCHDOG_DEBUG', 7);

/**
 * Log message severity -- Deprecated function/feature notices.
 */
define('BEE_WATCHDOG_DEPRECATED', 8);

/**
 * @} End of "defgroup bee_logging_severity_levels".
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
    $types = _bee_error_levels();
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
 * @param array $error
 *   An array with the following keys: !type, !message, !function, !file, !line
 *   and severity_level. All the parameters are plain-text.
 * @param bool $fatal
 *   TRUE if the error is fatal.
 *
 * @see _backdrop_log_error()
 */
function _bee_log_error(array $error, $fatal = FALSE) {

  // $severity_level is one of the BEE_WATCHDOG_* constants defined in
  // backdrop_error_levels. Map this to a bee_message type.
  switch ($error['severity_level']) {
    case BEE_WATCHDOG_ERROR:
      $type_info = bee_message_type_info('error');
      break;

    case BEE_WATCHDOG_WARNING:
      $type_info = bee_message_type_info('warning');
      break;

    case BEE_WATCHDOG_NOTICE:
      $type_info = bee_message_type_info('info');
      break;

    case BEE_WATCHDOG_DEBUG:
      $type_info = bee_message_type_info('log');
      break;
  }

  echo "\n " . bee_format_text($type_info[0], $type_info[1], TRUE) . ' ' . bee_format_text(bt("!type: !message\nin !function (line !line of !file).", $error)) . "\n\n";

  if ($fatal) {
    exit();
  }
}

/**
 * Gets the last caller from a backtrace.
 *
 * @param array $backtrace
 *   A standard PHP backtrace.
 *
 * @return array
 *   An associative array with keys 'file', 'line' and 'function'.
 *
 * @see _backdrop_get_last_caller()
 */
function _bee_get_last_caller(array $backtrace) {
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

/**
 * Maps PHP error constants to watchdog severity levels. Bee's own
 * implementation of backdrop_error_levels() in case Backdrop isn't yet
 * available.
 *
 * The error constants are documented at
 * http://php.net/manual/errorfunc.constants.php
 *
 * @ingroup bee_logging_severity_levels
 * @see backdrop_error_levels()
 */
function _bee_error_levels() {
  $types = array(
    E_ERROR => array('Error', BEE_WATCHDOG_ERROR),
    E_WARNING => array('Warning', BEE_WATCHDOG_WARNING),
    E_PARSE => array('Parse error', BEE_WATCHDOG_ERROR),
    E_NOTICE => array('Notice', BEE_WATCHDOG_NOTICE),
    E_CORE_ERROR => array('Core error', BEE_WATCHDOG_ERROR),
    E_CORE_WARNING => array('Core warning', BEE_WATCHDOG_WARNING),
    E_COMPILE_ERROR => array('Compile error', BEE_WATCHDOG_ERROR),
    E_COMPILE_WARNING => array('Compile warning', BEE_WATCHDOG_WARNING),
    E_USER_ERROR => array('User error', BEE_WATCHDOG_ERROR),
    E_USER_WARNING => array('User warning', BEE_WATCHDOG_WARNING),
    E_USER_NOTICE => array('User notice', BEE_WATCHDOG_NOTICE),
    E_STRICT => array('Strict warning', BEE_WATCHDOG_DEBUG),
    E_RECOVERABLE_ERROR => array('Recoverable fatal error', BEE_WATCHDOG_ERROR),
    E_DEPRECATED => array('Deprecated function', BEE_WATCHDOG_DEBUG),
    E_USER_DEPRECATED => array('User deprecated function', BEE_WATCHDOG_DEBUG),
  );

  return $types;
}
