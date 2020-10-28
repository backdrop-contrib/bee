<?php
/**
 * @file
 * Functions for dealing with user input from the terminal.
 */

/**
 * Ask the user the confirm a choice (i.e. a yes/no question).
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
    '#color' => 'cyan',
    '#bold' => TRUE,
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

  // Process user input.
  switch ($answer) {
    case 'y':
      return TRUE;
    case 'n':
      return FALSE;
    case '':
      return $default;
    default:
      b_render_text(array(
        'value' => bt("Invalid input. Please enter either 'y' or 'n', or press Enter to accept the default."),
        '#color' => 'red',
      ));
      return b_confirm($question, $default);
  }
}

/**
 * Ask the user to select an option from a list.
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
function b_choice($options, $message, $default = NULL) {
  global $yes_mode;
  $keys = array_keys($options);
  $rows = array();
  $default_answer = bt('Cancel');

  // Set the 'Cancel' option.
  $cancel_option = array(
    array(
      'value' => ' 0 ',
    ),
    array(
      'value' => bt('Cancel'),
    ),
  );
  if ($default === NULL) {
    $cancel_option[0]['value'] = '[0]';
  }
  $rows[] = $cancel_option;

  // Set all other options.
  foreach ($keys as $number => $key) {
    $row = array(
      array(
        'value' => ' ' . ($number + 1) . ' ',
      ),
      array(
        'value' => $options[$key],
      ),
    );
    if ($default === $key) {
      $row[0]['value'] = '[' . ($number + 1) . ']';
      $default_answer = $options[$key];
    }
    $rows[] = $row;
  }

  // Display the message and options.
  b_render_text(array(
    'value' => $message,
    '#color' => 'cyan',
    '#bold' => TRUE,
  ), TRUE);
  echo "\n";
  b_render(array(
    '#type' => 'table',
    'rows' => $rows,
    'delimiter' => ' ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  ), TRUE);
  b_render_text(array(
    'value' => bt('Enter a number: '),
  ), FALSE);

  // Return the default answer if `yes_mode` is enabled.
  if ($yes_mode) {
    b_render_text(array(
      'value' => $default_answer,
    ));
    return $default;
  }

  // Get answer from user.
  $answer = trim(fgets(STDIN));

  // Process user input.
  if (is_numeric($answer)) {
    if (isset($keys[$answer - 1])) {
      return $keys[$answer - 1];
    }
    elseif ($answer == 0) {
      return NULL;
    }
  }
  elseif (empty($answer)) {
    return $default;
  }

  // Prompt the user for input again.
  b_render_text(array(
    'value' => bt("Invalid input. Please enter a number corresponding to the options above, or press Enter to accept the [default] option."),
    '#color' => 'red',
  ));
  echo "\n";
  return b_choice($options, $message, $default);
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
