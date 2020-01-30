<?php
/**
 * @file
 * Functions for rendering elements.
 */

/**
 * Process an array of elements for rendering.
 *
 * @param array $elements
 *   An associative array which may contain:
 *   - #type: A string that determines which render function to call for
 *     formatting and displaying the element. One of: 'text', 'table'.
 *     See the appropriate render function for other key => value requirements.
 *   If there is no `#type` key, or if there is an `elements` key, sub-elements
 *   will be processed recursively.
 */
function b_render($elements) {
  // Ignore empty elements.
  if (empty($elements)) {
    return;
  }

  // For non-arrays, convert objects to arrays and print anything else as-is.
  if (!is_array($elements)) {
    if (is_object($elements)) {
      $elements = (array) $elements;
    }
    else {
      b_render_text($elements);
      return;
    }
  }

  // Recurse through the array to find the elements to render.
  if (isset($elements['#type'])) {
    $type = $elements['#type'];
    $function = 'b_render_' . $type;

    if (function_exists($function)) {
      $function($elements);
    }

    if (isset($elements['elements'])) {
      b_render($elements['elements']);
    }
  }
  else {
    foreach ($elements as $element) {
      b_render($element);
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
 *
 * @see b_format_text()
 */
function b_render_text($variables, $newline = TRUE) {
  $color = FALSE;
  $bold = FALSE;

  // Get the text and format options.
  if (is_array($variables)) {
    $text = isset($variables['value']) ? $variables['value'] : '';
    $color = isset($variables['#color']) ? $variables['#color'] : FALSE;
    $bold = isset($variables['#bold']) ? $variables['#bold'] : FALSE;
  }
  else {
    $text = (string) $variables;
  }

  // Format and output the text.
  echo b_format_text($text, $color, $bold);
  if ($newline) {
    echo "\n";
  }
}

/**
 * Format text to be displayed in the terminal.
 *
 * @param string|int|false $color
 *   The name or code of the color to use, or FALSE to use the default color.
 * @param bool $bold
 *   Whether or not to format the text as bold. Defaults to FALSE.
 *
 * @return string
 *   The formatted text.
 */
function b_format_text($text, $color = FALSE, $bold = FALSE) {
  // Get color codes.
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
    if (is_string($color) && isset($colors[$color])) {
      $code[] = $colors[$color];
    }
    elseif (is_int($color)) {
      $code[] = $color;
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
 * Count column width to fit screen width.
 */
function _render_table_count_columns_width($rows, $header, $delimiter, $delimiter_left, $delimiter_right) {
  $cols_length = array();

  if (is_array($header)) {
    $index = 0;
    foreach ($header as $col) {
      if (is_array($col) && isset($col['data'])) {
        $length = mb_strlen($col['data']);
      }
      else {
        $length = mb_strlen($col);
      }
      if (!isset($cols_length[$index]) or $cols_length[$index] < $length) {
        $cols_length[$index] = $length;
      }
      $index++;
    }
  }
  foreach ($rows as $row) {
    $index = 0;
    foreach ($row as $col) {
      if (is_array($col) && isset($col['data'])) {
        $length = mb_strlen($col['data']);
      }
      else {
        $length = mb_strlen($col);
      }
      if (!isset($cols_length[$index]) or $cols_length[$index] < $length) {
        $cols_length[$index] = $length;
      }
      $index++;
    }
  }

  $max_size_index = 0;
  $max_length = 0;
  $total_table_width = 0;
  foreach ($cols_length as $index => $length) {
    if ($length > $max_length) {
      $max_size_index = $index;
      $max_length = $length;
    }
    $total_table_width += $length;
  }

  $total_table_width += mb_strlen($delimiter_left);
  $total_table_width += mb_strlen($delimiter_right);
  $total_table_width += mb_strlen($delimiter) * count($cols_length) - 1;

  $max_width = exec('tput cols');
  if ($max_width && $total_table_width > $max_width) {
    $diff = $total_table_width - $max_width;
    $cols_length[$max_size_index] = $cols_length[$max_size_index] - $diff;
  }
  return $cols_length;
}

/**
 * Return column text based on width.
 */
function _render_print_col($col, $width) {
  $color_width = 0;
  if (is_array($col) && isset($col['data'])) {
    $data = b_format_text($col['data'], $col['#color'], $col['#bold']);
    $color_width = mb_strlen($data) - mb_strlen($col['data']);
  }
  else {
    $data = $col;
  }
  return sprintf('%-' . ($width + $color_width) . 's', $data);
}

/**
 * Display a table of information.
 *
 * @param array $variables
 *   An associative array containing:
 *   - rows: An array of columns. Each column is either text to output or an
 *     associative array containing:
 *     - data: The text to output.
 *     - #color: (optional) The name or code of the color to use for the text.
 *     - #bold: (optional) A boolean indicating whether or not to format the
 *       text as bold.
 *  - delimiter: The character(s) used to separate columns.
 *  - delimiter_left: The character(s) displayed on the left of the table.
 *  - delimiter_right: The character(s) displayed on the right of the table.
 */
function b_render_table($variables) {
  $header = FALSE;
  if (isset($variables['header'])) {
    $header = $variables['header'];
  }
  $rows = $variables['rows'];
  $delimiter = isset($variables['delimiter']) ? $variables['delimiter'] : " | ";
  $delimiter_left = isset($variables['delimiter_left']) ? $variables['delimiter_left'] : "| ";
  $delimiter_right = isset($variables['delimiter_right']) ? $variables['delimiter_right'] : " |";

  $cols_length = _render_table_count_columns_width($rows, $header, $delimiter, $delimiter_left, $delimiter_right);

  if ($header) {
    $row_rendered = array();
    foreach ($header as $index => $col) {
      $row_rendered[] = _render_print_col($col, $cols_length[$index]);
    }
    // Flip background and foreground colors for table header.
    $text = $delimiter_left . implode($delimiter, $row_rendered) . $delimiter_right;
    echo b_format_text($text, 7) . "\n";
  }

  foreach ($rows as $row) {
    $print_args = array();
    $repeat = FALSE;
    $row_rendered = array();
    foreach ($row as $index => $col) {

      $data = '';
      $col_len = 0;
      if (is_array($col) && isset($col['data'])) {
        if (mb_strlen($col['data']) > $cols_length[$index]) {
          $repeat = $col;
          $repeat['index'] = $index;
          $repeat['data'] = mb_substr($col['data'], $cols_length[$index]);
          $col['data'] = mb_substr($col['data'], 0, $cols_length[$index]);
        }
      }
      else {
        if (mb_strlen($col) > $cols_length[$index]) {
          $repeat = array(
            'index' => $index,
            'data' => mb_substr($col, $cols_length[$index]),
          );
          $col = mb_substr($col, 0, $cols_length[$index]);
        }
      }
      $row_rendered[] = _render_print_col($col, $cols_length[$index]);
    }
    echo $delimiter_left . implode($delimiter, $row_rendered) . $delimiter_right . "\n";
    if ($repeat) {
      $rows_return = _row_next_line_value($repeat, $cols_length, $delimiter);
      foreach ($rows_return as $row_return) {
        echo $delimiter_left . implode($delimiter, $row_return) . $delimiter_right . "\n";
      }
    }
  }
}

/**
 * Output next value line to fit column size.
 *
 * If column does not fit screen size and column size, we use this function to
 * properly output next value line to fit column size.
 */
function _row_next_line_value($repeat, $cols_length) {
  $row_rendered = array();
  $rows_return = array();
  foreach ($cols_length as $index => $length) {
    if ($index == $repeat['index']) {
      $col = $repeat;
      if (mb_strlen($col['data']) > $length) {
        $col['data'] = mb_substr($col['data'], 0, $length);
        $repeat['data'] = mb_substr($repeat['data'], $length);
      }
      else {
        $repeat = FALSE;
      }
      $row_rendered[$index] = _render_print_col($col, $length);
    }
    else {
      $row_rendered[$index] = _render_print_col('', $length);
    }
  }
  $rows_return[] = $row_rendered;
  if ($repeat) {
    $tmp = _row_next_line_value($repeat, $cols_length);
    foreach ($tmp as $tmp_row) {
      $rows_return[] = $tmp_row;
    }
  }

  return $rows_return;
}
