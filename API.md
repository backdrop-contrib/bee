API Information
===============

HOOK_b_command()
----------------
This hook can be invoked to provide additional commands to Backdrop Console. It
should reside in a `b` command file: `HOOK.b.inc`.

Implementations of this hook should return an associative array of command
descriptors, where the keys are unique command names and the values are
associative arrays containing:
- **description**: The translated description of the command.
- **callback**: The name of the function that runs the command.
- **arguments**: (optional) An array of required arguments for the command,
  where the keys are argument names and the values are translated argument
  descriptions. In addition, the key `#multiple` can be used with a value
  specifying an argument name that accepts multiple values. For example, set
  `'#multiple' => 'modules'` to specify that the 'modules' argument accepts
  multiple values.
- **options**: (optional) An array of options for the command, where the keys
  are option names (these will be prepended with '--' when displayed to the
  user) and the values are associative arrays containing:
  - **description**: The translated option description.
  - **value**: (optional) A translated word describing the value a user needs to
    provide for this option. This will be displayed to the user underlined and
    in uppercase after the option name.
- **aliases**: (optional) An array of alternate command names.
- **bootstrap**: (optional) The bootstrap level required to run this command.
  See `includes/globals.inc` for possible values.
- **examples**: (optional) An array of example use-cases for the command, where
  the keys are command strings (including `b`, options, arguments, etc.) and the
  values are translated explanations of the command string.

Example:
```php
function poetry_b_command() {
  return array(
    'poem' => array(
      'description' => bt('Displays a customised poem.'),
      'callback' => 'poem_callback',
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
        ),
      ),
      'aliases' => array('p'),
      // Backdrop is not actually needed for this command.
      // 'bootstrap' => B_BOOTSTRAP_CONFIGURATION,
      'examples' => array(
        'b poem HAL' => bt('Display a poem about HAL.'),
        'b poem --roses=Yellow Sarah' => bt('Display a poem about Sarah with yellow roses.'),
        'b poem --short Bob' => bt('Display a short poem about Bob.'),
      ),
    ),
  );
}
```

HOOK_callback()
---------------
This function is called when the user runs the given command (see
HOOK_b_command()). While it technically can be named anything at all, it's
recommended to adhere to the suggested `HOOK_callback()` format.

This callback function will receive two parameters:
- **$arguments**: An associative array where the keys are argument names and the
  values are user-provided values for those arguments. In the case where an
  argument is allowed multiple values, an array of user-provided values is
  passed.
- **$options**: An associative array where the keys are option names (*not*
  aliases) and the values are user-provided values for those options.

The function can optionally return an element, or array of elements, to render.
An element is an associative array containing:
- **#type**: A string that determines which render function to call for
  formatting and displaying the element. It should be one of: 'text', 'table'.
- Other keys/values
*     See the appropriate render function for other `key => value`
*     requirements.
*   If there is no `#type` key, or if there is an `elements` key, sub-elements
*   will be processed recursively.

Example:
```php
function poem_callback($arguments, $options) {
  // Get variables.
  $name = $arguments['name'];
  $colour = !empty($options['roses']) ? strtolower($options['roses']) : 'red';
  $short = isset($options['short']);

  // Display poem.
  if (!$short) {
    $poem = "Roses are $colour\n
      Violets are blue\n
      My name is $name\n
      How about you?\n";
  }
  else {
    $poem = "$name is my name and poetry's my game!\n";
  }
  echo $poem;
}
```
