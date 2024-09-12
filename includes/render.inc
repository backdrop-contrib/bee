<?php
/**
 * @file
 * Functions for rendering elements.
 */

/**
 * Format and display all post operation messages to the user.
 */
function bee_print_messages() {
  global $_bee_messages, $_bee_debug_mode;
  $rows = array();
  // Exit if there are no messages.
  if (empty($_bee_messages)) {
    return;
  }

  // Add a new line before starting.
  echo "\n";

  // Style each type of message differently.
  foreach ($_bee_messages as $type => $type_messages) {
    // Skip 'log' messages if `debug` mode is disabled.
    if ($type == 'log' && !$_bee_debug_mode) {
      continue;
    }

    list($symbol, $color) = bee_message_type_info($type);

    // Output the symbol and text for each message.
    foreach ($type_messages as $message) {
      echo bee_format_text(" $symbol", $color) . " $message";

      // Add a newline after each message.
      echo "\n";
    }
  }

  // Add a new line after the messages.
  echo "\n";
}

/**
 * Return the symbol and color for a bee message type.
 *
 * @param string $type
 *   A string that identifies the type of message.
 *
 * @return array
 *   An array of two elements, a textual symbol and a string identifying its
 *   color.
 */
function bee_message_type_info($type) {
  switch ($type) {
    case 'success':
      $symbol = BEE_SYMBOL_SUCCESS;
      $color = 'green';
      break;

    case 'warning':
      $symbol = BEE_SYMBOL_WARNING;
      $color = 'yellow';
      break;

    case 'error':
      $symbol = BEE_SYMBOL_ERROR;
      $color = 'red';
      break;

    case 'log':
      $symbol = BEE_SYMBOL_LOG;
      $color = 'magenta';
      break;

    case 'debug':
      $symbol = BEE_SYMBOL_DEBUG;
      $color = '92';
      break;

    case 'info':
    default:
      $symbol = BEE_SYMBOL_INFO;
      $color = 'blue';
  }
  return array($symbol, $color);
}

/**
 * Display the output of the given command.
 */
function bee_display_output() {
  global $_bee_output;
  if (!is_array($_bee_output)) {
    return;
  }

  // Process each element in the output.
  foreach ($_bee_output as $element) {
    if (empty($type = $element['type']) || empty($variables = $element['variables'])) {
      continue;
    }
    $newline = isset($element['newline']) ? $element['newline'] : TRUE;

    // Call the element's render function.
    $function = "bee_render_$type";
    if (function_exists($function)) {
      $function($variables, $newline);
    }
  }
}

/**
 * Output formatted text.
 *
 * @param array $variables
 *   An associative array containing:
 *   - value: The text to format and output.
 *   - #color: (optional) The name or code of the color to use for the text.
 *   - #bold: (optional) A boolean indicating whether or not to format the text
 *     as bold.
 * @param bool $newline
 *   Whether or not to add a newline character (`\n`) to the end of the output.
 *   Defaults to TRUE.
 */
function bee_render_text(array $variables, $newline = TRUE) {
  // Get the text and format options.
  $text = $variables['value'];
  $color = isset($variables['#color']) ? $variables['#color'] : FALSE;
  $bold = isset($variables['#bold']) ? $variables['#bold'] : FALSE;

  // Format and output the text.
  echo bee_format_text($text, $color, $bold);
  if ($newline) {
    echo "\n";
  }
}

/**
 * Output information and messages as the code happens.
 *
 * In many ways, this function is similar to 'bee_message()' but there are some
 * key differences:
 *   - This function will output the message and optional data at the point in
 *     the code when it happens rather than at the end.
 *   - It includes the ability to output an array of data after the message for
 *     'debug' messages.
 *   - It includes the calling function and line number for 'debug' messages.
 *
 * WARNING: If you insert a call to this function in the core bee code before
 *          a command that requires Backdrop to be boostrapped, it will produce
 *          'Headers already sent' warnings so only use temporarily for debug
 *          purposes.
 *
 * @param string $message
 *   The translated message to display.
 * @param string $type
 *   The type of message to display (this determines the formatting used). One
 *   of: info, success, warning, error, log, debug. Defaults to 'info'.
 *   Note that 'log' and 'debug' messages are only displayed when `debug` mode
 *   is enabled.
 * @param array $data
 *   An optional array of data that will be outputted after the message for
 *   'debug' types only (Ignored for other types). Defaults to empty array.
 */
function bee_instant_message($message, $type = 'info', array $data = array()) {
  global $_bee_debug_mode;

  // Exit the function if not debug mode and type is 'log' or 'debug'.
  if (!$_bee_debug_mode && in_array($type, array('log','debug'))) {
    return;
  }

  // Add the symbol.
  $symbol = bee_message_type_info($type);
  echo ' ' . bee_format_text($symbol[0], $symbol[1]);

  // If debug mode and type is 'debug' get the calling information.
  if ($_bee_debug_mode && $type == 'debug') {
    // Get the calling function and line. Need to get two functions.
    $backtrace = debug_backtrace(DEBUG_BACKTRACE_IGNORE_ARGS, 2);
    $function = $backtrace[1]['function'];
    $line = $backtrace[0]['line'];

    // Output the function name and line it is called from.
    echo " [$function:$line]";
  }

  // Output the string.
  echo " $message";

  // Output newline.
  echo "\n";

  // If debug mode AND type is 'debug' AND data array is populated, output the
  // array.
  if ($_bee_debug_mode && $type == 'debug' && !empty($data)) {
    // phpcs:ignore Squiz.PHP.DiscouragedFunctions -- integral part of the command
    print_r($data);
    echo "\n";
  }
}

/**
 * Format text to be displayed in the terminal.
 *
 * @param string $text
 *   The text to be formatted.
 * @param string|false $color
 *   The name or code of the color to use, or FALSE to use the default color.
 * @param bool $bold
 *   Whether or not to format the text as bold. Defaults to FALSE.
 *
 * @return string
 *   The formatted text.
 */
function bee_format_text($text, $color = FALSE, $bold = FALSE) {
  // Get color codes.
  // @see https://en.wikipedia.org/wiki/ANSI_escape_code#3/4_bit
  $colors = array(
    'black' => '30',
    'red' => '31',
    'green' => '32',
    'yellow' => '33',
    'blue' => '34',
    'magenta' => '35',
    'cyan' => '36',
    'gray' => '37',
    'white' => '97',
  );

  // Generate format code.
  $code = array();
  if ($bold) {
    $code[] = '1';
  }
  if (!empty($color)) {
    if (is_numeric($color)) {
      $code[] = $color;
    }
    elseif (isset($colors[$color])) {
      $code[] = $colors[$color];
    }
  }

  // Return formatted text.
  if (!empty($code)) {
    return "\033[" . implode(';', $code) . 'm' . $text . "\033[0m";
  }
  else {
    return $text;
  }
}

/**
 * Output a table of information.
 *
 * @param array $variables
 *   An associative array containing:
 *   - rows: An array of rows that make up the table, each of which is an array
 *     of columns. Each column is an associative array containing:
 *     - value: The text to format and output.
 *     - #color: (optional) The name or code of the color to use for the text.
 *     - #bold: (optional) A boolean indicating whether or not to format the
 *       text as bold.
 *   - header: (optional) An array of columns to display as the first row of the
 *     table. Each column is an associative array as described in `rows` above.
 *   - delimiter: (optional) The character(s) used to separate columns.
 *   - delimiter_left: (optional) The character(s) displayed on the left of the
 *     table.
 *   - delimiter_right: (optional) The character(s) displayed on the right of
 *     the table.
 * @param bool $newline
 *   Whether or not to add a newline character (`\n`) to the end of the output.
 *   Defaults to TRUE.
 */
function bee_render_table(array $variables, $newline = TRUE) {
  // Get variables and set default values.
  $rows = $variables['rows'];
  $header = isset($variables['header']) ? $variables['header'] : array();
  $delimiter = isset($variables['delimiter']) ? $variables['delimiter'] : ' | ';
  $delimiter_left = isset($variables['delimiter_left']) ? $variables['delimiter_left'] : '| ';
  $delimiter_right = isset($variables['delimiter_right']) ? $variables['delimiter_right'] : ' |';

  // Get the width of each column.
  $column_widths = bee_get_column_widths($rows, $header, $delimiter, $delimiter_left, $delimiter_right);

  // Print the header.
  if (!empty($header)) {
    $rendered_row = array();

    foreach ($header as $index => $column) {
      $rendered_row[] = bee_format_column($column, $column_widths[$index]);
    }

    // Invert the colors of the header.
    bee_render_text(array(
      'value' => $delimiter_left . implode($delimiter, $rendered_row) . $delimiter_right,
      '#color' => '7',
    ));
  }

  // Print each row.
  foreach ($rows as $row) {
    $rendered_row = array();
    $remaining = FALSE;

    foreach ($row as $index => $column) {
      // Allow for text that is wider than the column width.
      if (mb_strlen((string) $column['value']) > $column_widths[$index]) {
        $remaining = $column;
        $remaining['index'] = $index;
        $remaining['value'] = mb_substr($column['value'], $column_widths[$index]);

        $column['value'] = mb_substr($column['value'], 0, $column_widths[$index]);
      }

      $rendered_row[] = bee_format_column($column, $column_widths[$index]);
    }

    bee_render_text(array('value' => $delimiter_left . implode($delimiter, $rendered_row) . $delimiter_right));

    if ($remaining) {
      $remaining_rows = bee_get_remaining_rows($remaining, $column_widths);
      foreach ($remaining_rows as $remaining_row) {
        bee_render_text(array('value' => $delimiter_left . implode($delimiter, $remaining_row) . $delimiter_right));
      }
    }
  }

  if ($newline) {
    echo "\n";
  }
}

/**
 * Calculate a table's column widths and make sure they fit in the terminal.
 *
 * @see bee_render_table()
 *
 * @return array
 *   An associative array of column widths, where the keys are column indexes
 *   (numbered from 0) and the values are widths (in number of characters).
 */
function bee_get_column_widths($rows, $header, $delimiter, $delimiter_left, $delimiter_right) {
  $widths = array();

  // Calculate the width of each column in the header.
  if (!empty($header)) {
    foreach ($header as $index => $column) {
      $widths[$index] = mb_strlen((string) $column['value']);
    }
  }

  // Calculate the width of each column in each row.
  foreach ($rows as $row) {
    foreach ($row as $index => $column) {
      $width = mb_strlen((string) $column['value']);
      if (!isset($widths[$index]) || $widths[$index] < $width) {
        $widths[$index] = $width;
      }
    }
  }
  bee_instant_message(bt('Column widths:'), 'debug', $widths);

  // Get the widest column, and calculate the width of the table.
  $widest_column = 0;
  $table_width = 0;
  foreach ($widths as $index => $width) {
    if ($width > $widths[$widest_column]) {
      $widest_column = $index;
    }
    $table_width += $width;
  }
  $table_width += mb_strlen((string) $delimiter_left);
  $table_width += mb_strlen((string) $delimiter_right);
  $table_width += (count($widths) - 1) * mb_strlen((string) $delimiter);

  // Adjust the width of the widest column to fit in the terminal. If a command
  // outputting a table is run in a non-interactive shell (e.g. as part of an
  // SSH command) then 'tput cols' on its own will create an error. This
  // provides an alternative if 'tput cols' create an error.
  $terminal_width = exec('if tput cols &>/dev/null; then  echo $(tput cols); else $(echo $COLUMNS); fi');
  bee_instant_message(bt('The terminal width is: ') . $terminal_width, 'debug');
  if ($terminal_width && $table_width > $terminal_width) {
    $diff = $table_width - $terminal_width;
    $widths[$widest_column] -= $diff;
  }

  return $widths;
}

/**
 * Format a column ready to insert in a table row.
 *
 * @param array $column
 *   An associative array containing:
 *   - value: The text to format and output in the column.
 *   - #color: (optional) The name or code of the color to use for the text.
 *   - #bold: (optional) A boolean indicating whether or not to format the text
 *     as bold.
 * @param int $width
 *   The width to set for the column.
 *
 * @return string
 *   The formatted string for the column.
 */
function bee_format_column(array $column, $width) {
  // Get the text and format options.
  $text = $column['value'];
  $color = isset($column['#color']) ? $column['#color'] : FALSE;
  $bold = isset($column['#bold']) ? $column['#bold'] : FALSE;

  $formatted_text = bee_format_text($text, $color, $bold);
  $color_width = mb_strlen((string) $formatted_text) - mb_strlen((string) $column['value']);

  return sprintf('%-' . ($width + $color_width) . 's', $formatted_text);
}

/**
 * Get the remaining rows yet to be output when a column is too long to fit in
 * the terminal.
 *
 * @param array $row
 *   The remains of the row to be output.
 * @param array $column_widths
 *   An associative array of column widths keyed by index.
 *   @see bee_get_column_widths()
 *
 * @return array
 *   An array of rows to add to the table.
 */
function bee_get_remaining_rows(array $row, array $column_widths) {
  $rows = array();
  $row_rendered = array();

  foreach ($column_widths as $index => $width) {
    if ($row && $index == $row['index']) {
      $column = $row;

      if (mb_strlen((string) $column['value']) > $width) {
        $row['value'] = mb_substr($row['value'], $width);
        $column['value'] = mb_substr($column['value'], 0, $width);
      }
      else {
        $row = FALSE;
      }

      $row_rendered[$index] = bee_format_column($column, $width);
    }
    else {
      $row_rendered[$index] = bee_format_column(array('value' => ''), $width);
    }
  }

  $rows[] = $row_rendered;

  if ($row) {
    $remaining_rows = bee_get_remaining_rows($row, $column_widths);
    foreach ($remaining_rows as $remaining_row) {
      $rows[] = $remaining_row;
    }
  }

  return $rows;
}
