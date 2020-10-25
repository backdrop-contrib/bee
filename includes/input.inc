<?php
/**
 * @file
 * Functions for dealing with user input from the terminal.
 */

/**
 * Asks the user the confirm a choice (i.e. a yes/no question).
 *
 * @param string $question
 *   The question to ask the user.
 * @param bool $default
 *   The default answer (shown to the user in uppercase). Defaults to FALSE.
 *
 * @return bool
 *   TRUE if the user enters "y", FALSE if "n".
 */
function b_confirm($question, $default = FALSE) {
  global $yes_mode;

  // Set the default option.
  if ($default) {
    $options = ' (Y/n): ';
  }
  else {
    $options = ' (y/N): ';
  }

  // Display the question.
  b_render_text(array(
    'value' => $question . $options,
    '#color' => 'magenta',
  ), FALSE);

  // Return TRUE if `yes_mode` is enabled.
  if ($yes_mode) {
    b_render_text(array(
      'value' => 'y',
    ));
    return TRUE;
  }

  // Get answer from user.
  $answer = strtolower(trim(fgets(STDIN)));

  switch ($answer) {
    case 'y':
      return TRUE;
      break;
    case 'n':
      return FALSE;
      break;
    case '':
      return $default;
      break;
    default:
      b_render_text(array(
        'value' => bt("Invalid input. Please enter either 'y' or 'n', or press Enter to accept the default."),
        '#color' => 'red',
      ));
      b_confirm($question, $default);
  }
}

/**
 * Asks the user to select an option from a list.
 *
 * @param array $options
 *   An associative array of choices to display to the user, where keys are
 *   return values (that will be sent back to the calling function) and values
 *   are translated choice names. When displayed to the user, each choice will
 *   be assigned a number from 1 to N, with 0 reserved for 'Cancel'.
 * @param string $message
 *   The message to display to the user, prompting for input.
 * @param mixed $default
 *   The default answer (shown to the user in brackets). Defaults to 'Cancel'.
 *
 * @return mixed
 *   The key (return value) of the selected choice.
 */
function b_choice($options, $message, $default = 0) {
  global $yes_mode;
  $keys = array_keys($options);
  $rows = array();

  // Set the 'Cancel' option.
  $cancel_option = array(
    array(
      'value' => ' 0 ',
      '#color' => 'red',
    ),
    array(
      'value' => bt('Cancel'),
    ),
  );
  if ($default === 0) {
    $cancel_option[0]['value'] = '[0]';
  }
  $rows[] = $cancel_option;

  // Set all other options.
  foreach ($keys as $number => $key) {
    $row = array(
      array(
        'value' => ' ' . ($number + 1) . ' ',
        '#color' => 'blue',
        '#bold' => TRUE,
      ),
      array(
        'value' => $options[$key],
      ),
    );
    if ($default === $key) {
      $row[0]['value'] = '[' . ($number + 1) . ']';
    }
    $rows[] = $row;
  }

  // Display the options and message.
  b_render(array(
    '#type' => 'table',
    'rows' => $rows,
    'delimiter' => ' ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  ), TRUE);
  b_render_text(array(
    'value' => $message . ': ',
    '#color' => 'magenta',
  ), FALSE);

  // Automatically accept confirmations if the --yes argument was supplied.
  // if ($yes_mode) {
  //   b_render_text("y");
  //   return $keys[0];
  // }
  //
  // $stdin = fopen("php://stdin", "r");
  //
  // while ($line = fgets($stdin)) {
  //   $line = trim($line);
  //   $num = (int) $line;
  //
  //   if ($num == "") {
  //     return FALSE;
  //   }
  //   if ($num == 'n') {
  //     return FALSE;
  //   }
  //   if ($num == 0) {
  //     return FALSE;
  //   }
  //
  //   if (isset($keys[$num - 1])) {
  //     return $keys[$num - 1];
  //   }
  //
  //   render_text($text, FALSE);
  // }
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
