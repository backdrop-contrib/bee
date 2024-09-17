# API Information
Source: https://github.com/backdrop-contrib/bee/blob/1.x-1.x/API.md

## `HOOK_bee_command()`
This hook can be invoked to provide additional commands to Bee. It should reside
in a `bee` command file (*HOOK.bee.inc*) which should be placed in a
custom/contrib module, in the `.bee` folder in the user's HOME directory or in
the `.bee` folder in the parent folder of the Backdrop root directory.

Implementations of this hook should return an associative array of command
descriptors, where the keys are unique command names and the values are
associative arrays, containing:

- **description**: The translated description of the command.
- **callback**: The name of the function that runs the command. Should be of the
  form `COMMAND_bee_callback`.
- **command_requirements**: (optional) An array of the following requirements
  to override the defaults:-
  - `backdrop_root` - whether the command must be run inside a Backdrop root or
  using the `--root` global option.
  - `backdrop_installed` - whether Backdrop must be installed before the
   command can be run.

  Both have a default value of TRUE and if you need to override one or both,
  include both values in your command.
- **group**: The name of the group the command belongs to (used when listing all
  commands together). Group names should be lowercase with individual words
  separated by an underscore. See `bee help` for a list of existing groups.
- **arguments**: (optional) An array of required arguments for the command,
  where the keys are argument names and the values are translated argument
  descriptions.
- **multiple_argument**: (optional) The argument name that accepts multiple
  values. Note the singular form of this key - only a single argument can have
  multiple values.
- **optional_arguments**: (optional) Generally, arguments are required
  parameters and options are optional ones, but on the rare occasion that
  optional argument(s) are needed, an array of argument names can be specified
  here.
- **options**: (optional) An array of options for the command, where the keys
  are option names (these will be prepended with '--' when displayed to the
  user) and the values are associative arrays containing:
  - **description**: The translated option description.
  - **value**: (optional) A translated word describing the value a user needs to
    provide for this option. This will be displayed to the user in uppercase
    after the option name.
  - **short**: (optional) A short code for an option that does not require a
    value. These will be prepended with '-' when displayed to the user. Any
    short options are converted to the full option before being passed to the
    command callback so you only need to check for the full option in your
    function.
- **aliases**: (optional) An array of alternate command names.
- **bootstrap**: (optional) The bootstrap level required to run this command.
  See *includes/globals.inc* for possible values.
- **examples**: (optional) An array of example use-cases for the command, where
  the keys are command strings (including `bee`, options, arguments, etc.) and
  the values are translated explanations of the command string.

Example:

```php
function poetry_bee_command() {
  return array(
    'poem' => array(
      'description' => bt('Displays a customised poem.'),
      'callback' => 'poem_bee_callback',
      'command_requirements' => array(
        'backdrop_root' => FALSE,
        'backdrop_installed' => FALSE,
      ),
      'group' => 'poetry',
      'arguments' => array(
        'name' => bt('The name to use in the poem.'),
      ),
      'options' => array(
        'roses' => array(
          'description' => bt('Make roses a different colour.'),
          'value' => bt('Colour'),
        ),
        'short' => array(
          'description' => bt('Display a shorter poem.'),
          'short' => 's',
        ),
      ),
      'aliases' => array('p'),
      'examples' => array(
        'bee poem HAL' => bt('Display a poem about HAL.'),
        'bee poem --roses=Yellow Sarah' => bt('Display a poem about Sarah with yellow roses.'),
        'bee poem --short Bob' => bt('Display a short poem about Bob.'),
      ),
    ),
  );
}
```

## `COMMAND_bee_callback()`
This function is called when the user runs the given command (see
`HOOK_bee_command()`). It is highly recommended to adhere to the suggested
`COMMAND_bee_callback()` format to avoid collisions with other Backdrop function
names.

This callback function will receive two parameters:

- **$arguments**: An associative array where the keys are argument names and the
  values are user-provided values for those arguments. In the case where an
  argument is allowed multiple values, an array of user-provided values is
  passed.
- **$options**: An associative array where the keys are option names (*not*
  aliases) and the values are user-provided values for those options.

The function can optionally return an array of elements to render as output. An
element is an associative array, containing:

- **type**: A string that determines which render function to call for
  formatting and displaying the element. It should be one of: 'text', 'table'.
- **variables**: An array of variables to pass to the specific render function.
  See the individual `bee_render_TYPE()` functions in *includes/render.inc* for
  details about their individual parameters.
- **newline**: (optional) A boolean that determines whether or not to add a
  newline character (`\n`) to the end of the output. Defaults to TRUE.

Example:

```php
function poem_bee_callback($arguments, $options) {
  // Get variables.
  $name = $arguments['name'];
  $colour = !empty($options['roses']) ? strtolower($options['roses']) : 'red';
  $short = isset($options['short']);

  // Generate poem.
  if (!$short) {
    $poem = bt("Roses are !colour\nViolets are blue\nMy name is !name\nHow about you?\n", array(
      '!colour' => $colour,
      '!name' => $name,
    ));
  }
  else {
    $poem = bt("!name is my name and poetry's my game!\n", array(
      '!name' => $name,
    ));
  }

  // Return render array.
  return array(
    array(
      'type' => 'text',
      'variables' => array(
        'value' => $poem,
      ),
      'newline' => TRUE,
    ),
  );
}
```

## Helper functions
There are a number of helper functions that can be called to assist in
performing various tasks. Read the documentation for them in their respective
files.

- **`bee_message()`** (*includes/miscellaneous.inc*)  
  Any time a message needs to be shown to the user at completion of the
  operation, this function should be used. It collects all messages and then
  displays them to the user at the conclusion of the operation. A message, as
  opposed to regular text, has a type; being one of: status, success, warning,
  error or log. Note that 'log' messages are only displayed to the user when
  'debug' mode is enabled.

- **`bee_instant_message()`** (*includes/render.inc*)  
  If a message needs to be displayed to users as the code happens, this
  function should be used. This could be to output a message to say something
  is going to happen before it happens, for example, downloading a file or
  completing a database operation. It can also be used for more advanced
  debugging as the message will output at the time of code execution and
  and therefore can output information before a fatal error. It includes an
  additional message type of 'debug'.
  In many ways, this function is similar to 'bee_message()' but there are some
  key differences:
    - This function will output the message and optional data at the point in
      the code when it happens rather than at the end.
    - It includes the ability to output an array of data after the message for
      'debug' messages.
    - It includes the calling function and line number for 'debug' messages.

- **`bt()`** (*includes/miscellaneous.inc*)  
  All text that can be translated into other languages should be run through
  this function. This is the Bee equivalent of Backdrop's `t()` function.

- **`bee_get_temp()`** (*includes/filesystem.inc*)  
  If a temporary directory is needed (e.g. for downloading files, etc. before
  moving them to a more permanent location), this function will provide the path
  to the temporary directory.

- **`bee_delete()`** (*includes/filesystem.inc*)  
  This helper function will delete a file or directory from the filesystem. If a
  directory is specified, everything in that directory will be deleted in
  addition to the directory itself.

- **`bee_copy()`** (*includes/filesystem.inc*)  
  This helper function will copy a file or directory from one location to
  another. If a directory is specified as the source to be copied, everything in
  that directory will be copied as well.

- **`bee_render_text()`** (*includes/render.inc*)  
  If regular text (i.e. not a message) needs to be shown to the user, this
  function will allow it to be formatted and displayed. Note that any text
  displayed by calling this function directly will be shown before any messages,
  and before the final command output. As such, it is preferable to display text
  to the user using the regular command output instead (where appropriate).

- **`bee_format_text()`** (*includes/render.inc*)  
  This helper function assists in formatting text; such as using different
  colours and making the text bold. It can be used to format text that will be
  displayed later (for example, in the command output).

- **`bee_render_table()`** (*includes/render.inc*)  
  If a table of information (e.g. columns or tabular data) needs to be shown to
  the user, this function will allow it to be formatted and displayed. Note that
  any information displayed by calling this function directly will be shown
  before any messages, and before the final command output. As such, it is
  preferable to display information to the user using the regular command output
  instead (where appropriate).

- **`bee_confirm()`** (*includes/input.inc*)  
  This helper function will prompt the user to answer a yes/no question. This is
  useful, for example, when a command needs confirmation before performing
  certain, irreversible actions. If 'yes' mode is enabled, the affirmative
  option will be automatically chosen for the user.

- **`bee_choice()`** (*includes/input.inc*)  
  This helper function will prompt the user to select an option from a list of
  choices. Since the selection of an appropriate answer cannot be automated
  during the execution of the command, it is recommended that commands provide
  an option that the user can specify when running the command initially.

- **`bee_input()`** (*includes/input.inc*)  
  This helper function will prompt the user to enter a string of data. This is
  useful, for example, when the command needs information from the user that
  cannot be expressed as a yes/no question, or as a selection from a finite list
  of choices - e.g. the user's name or email address. Since the collection of
  this information cannot be automated during the execution of the command, it
  is recommended that commands provide an option that the user can specify when
  running the command initially.