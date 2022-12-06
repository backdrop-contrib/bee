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
function bee_confirm($question, $default = FALSE) {
  global $_bee_yes_mode;

  // Set the default option.
  if ($default) {
    $options = ' (Y/n): ';
  }
  else {
    $options = ' (y/N): ';
  }

  // Display the question.
  bee_render_text(array(
    'value' => $question . $options,
    '#color' => 'cyan',
    '#bold' => TRUE,
  ), FALSE);

  // Return TRUE if `yes_mode` is enabled.
  if ($_bee_yes_mode) {
    bee_render_text(array('value' => 'y'));
    return TRUE;
  }

  // Get answer from user.
  $answer = strtolower(trim(fgets(STDIN)));

  // Process user input.
  switch ($answer) {
    case 'y':
    case 'yes':
      return TRUE;
    case 'n':
    case 'no':
      return FALSE;
    case '':
      return $default;
    default:
      bee_render_text(array(
        'value' => bt("Invalid input. Please enter either 'y' or 'n', or press Enter to accept the default."),
        '#color' => 'red',
      ));
      return bee_confirm($question, $default);
  }
}

/**
 * Ask the user to select an option from a list.
 *
 * @param array $options
 *   An associative array of choices to display to the user, where keys are
 *   return values (that will be sent back to the calling function) and values
 *   are translated choice names. The first option should generally be a special
 *   one (e.g. Cancel or All).
 * @param string $message
 *   The message to display to the user, prompting for input.
 * @param mixed $default
 *   The array key of the default answer (shown to the user in brackets).
 * @param boolean $repeat
 *   Whether this choice is being reapeated (because of an invalid answer).
 *   Repeated choices just ask for input, rather than displaying the full list
 *   of options again. Defaults to FALSE.
 *
 * @return mixed
 *   The array key of the selected option.
 */
function bee_choice(array $options, $message, $default, $repeat = FALSE) {
  $keys = array_keys($options);
  $rows = array();

  // Set the options.
  foreach ($keys as $number => $key) {
    $row = array(
      array('value' => " $number "),
      array('value' => $options[$key]),
    );
    if ($default === $key) {
      $row[0]['value'] = "[$number]";
    }
    $rows[] = $row;
  }

  // Display the message and options.
  if (!$repeat) {
    bee_render_text(array(
      'value' => $message,
      '#color' => 'cyan',
      '#bold' => TRUE,
    ), TRUE);
    echo "\n";
    bee_render_table(array(
      'rows' => $rows,
      'delimiter' => ' ',
      'delimiter_left' => ' ',
      'delimiter_right' => ' ',
    ), TRUE);
  }
  bee_render_text(array('value' => bt('Enter a number: ')), FALSE);

  // Get answer from user.
  $answer = trim(fgets(STDIN));

  // Process user input.
  if (is_numeric($answer)) {
    $answer = (int) $answer;
    if (isset($keys[$answer])) {
      return $keys[$answer];
    }
  }
  elseif ($answer == '') {
    return $default;
  }

  // Prompt the user for input again.
  bee_render_text(array(
    'value' => bt('Invalid input. Please enter a number corresponding to the options above, or press Enter to accept the [default] option.'),
    '#color' => 'red',
  ));
  return bee_choice($options, $message, $default, TRUE);
}

/**
 * Ask the user to enter a string of data (e.g. their name or email address).
 *
 * @param string $message
 *   The message to display to the user, prompting for input. A colon ':' will
 *   be automatically appended to the end of this string.
 * @param string $default
 *   The default answer (shown to the user in brackets). Defaults to an empty
 *   string (i.e. no default value shown) - users can still just press Enter
 *   which will return an empty string (which may or may not be accepted,
 *   depending on the value of $required).
 * @param bool $required
 *   Whether or not the user is required to provide an answer. This is only
 *   taken into account when $default is also empty. If TRUE, an empty string
 *   from the user will not be accepted and the user will be prompted again.
 *   Defaults to FALSE.
 *
 * @return string
 *   The user's raw input, or the value of $default. NOTE: it is the calling
 *   functions job to sanitize this text appropriately prior to using it.
 */
function bee_input($message, $default = '', $required = FALSE) {
  // Add the default value in brackets (if any).
  if (!empty($default)) {
    $message .= " [$default]";
  }
  $message .= ': ';

  // Display the message.
  bee_render_text(array(
    'value' => $message,
    '#color' => 'cyan',
    '#bold' => TRUE,
  ), FALSE);

  // Get answer from user.
  $answer = trim(fgets(STDIN));

  // Process user input.
  if ($answer == '') {
    if ($default != '' || !$required) {
      return $default;
    }
  }
  else {
    return $answer;
  }

  // Prompt the user for input again.
  bee_render_text(array(
    'value' => bt('Invalid input. This answer is required and cannot be left empty. Please try again.'),
    '#color' => 'red',
  ));
  return bee_input($message, $default, TRUE);
}
