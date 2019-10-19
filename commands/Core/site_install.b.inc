<?php
/**
 * @file
 * Command(s) for installing Backdrop.
 */

/**
 * Implements hook_b_command().
 */
function site_install_b_command() {
  $items['site-install'] = array(
    'description' => bt('Install Backdrop along with modules/themes/configuration using the specified install profile.'),
    'arguments' => array(
      'profile' => bt('The install profile you wish to run. Default is "standard"'),
      'key=value' => array(
        'description' => bt('Any additional settings you wish to pass to the profile. The key is in the form [form name].[parameter name]'),
        'multiple' => TRUE,
      ),
    ),
    'options' => array(
      'root' => 'Set the working directory for the script to the specified path. Required if running this script from a directory other than the Backdrop root.',
      'db-url' => array(
        'description' => 'A Drupal 6 style database URL. Only required for initial install - not re-install.',
        'example-value' => 'mysql://root:pass@host/db',
        'required' => FALSE,
      ),
      'db-prefix' => 'An optional table prefix to use for initial install.  Can be a key-value array of tables/prefixes in a drushrc file (not the command line).',
      'account-name' => 'uid1 name. Defaults to admin',
      'account-pass' => 'uid1 pass. Defaults to a randomly generated password. If desired, set a fixed password in drushrc.php.',
      'account-mail' => 'uid1 email. Defaults to admin@example.com',
      'locale' => array(
        'description' => 'A short language code. Sets the default site language. Language files must already be present. You may use download command to get them.',
        'example-value' => 'en-GB',
      ),
      'clean-url' => 'Defaults to 1',
      'site-name' => 'Defaults to Site-Install',
      'site-mail' => 'From: for system mailings. Defaults to admin@example.com',
    ),
    'callback' => 'site_install_b_callback',
    'aliases' => array('si'),
  );
  return $items;
}

/**
 * Callback function for hook_b_command().
 *
 * @param array $arguments
 *   An array of user-provided arguments to the command.
 * @param array $options
 *   An array of user-provided options to the command.
 *
 * @return bool
 *   Whether or not Backdrop was installed successfully.
 */
function site_install_b_callback($arguments, $options) {

  $additional_form_options = array();
  if (isset($arguments['key=value']) && !empty($arguments['key=value'])) {
    foreach ($arguments['key=value'] as $argument) {
      list($key, $value) = explode('=', $argument);
      $additional_form_options[$key] = $value;
    }
  }

  $profile = 'standard';
  if (isset($arguments['profile'])) {
    $profile = $arguments['profile'];
  }

  // If the db-url is not provided get the information interactively from the
  // user.
  if (!isset($options['db-url'])) {
    // Prompt for site information.
    $default_options['account-mail'] = b_prompt('account-mail', 'admin@example.com');
    $default_options['account-name'] = b_prompt('account-name', 'admin');
    $default_options['account-pass'] = b_prompt('account-pass', md5(microtime() . mt_rand()));
    $default_options['site-name'] = b_prompt('site-name', 'Backdrop CMS');
    $default_options['clean-url'] = b_prompt('clean-url', '1');
    $default_options['db-prefix'] = b_prompt('db-prefix', "\'\'");
    $default_options['locale'] = b_prompt('locale', 'en');
    $default_options['site-mail'] = b_prompt('site-mail', 'admin@example.com');

    // Prompt for DB Specs.
    print bt("\n\tDB Credentials") . "\n";
    $db_spec['driver'] = 'mysql';
    $db_spec['username'] = b_prompt('username', 'your_db_user');
    $db_spec['password'] = b_prompt('password', 'your_db_password');
    $db_spec['port'] = b_prompt('port', '3306');
    $db_spec['host'] = b_prompt('host', 'localhost');
    $db_spec['database'] = b_prompt('database_name', 'database_name');
  }
  else {
    $default_options = array(
      'account-mail' => 'admin@example.com',
      'account-name' => 'admin',
      'account-pass' => md5(microtime() . mt_rand()),
      'site-name' => 'Backdrop',
      'clean-url' => '1',
      'db-prefix' => '',
      'db-url' => '',
      'locale' => 'en',
      'site-mail' => 'admin@example.com',
      'root' => '',
    );

    $url = parse_url($options['db-url']);
    $url += array(
      'driver' => NULL,
      'user' => NULL,
      'pass' => NULL,
      'host' => NULL,
      'port' => NULL,
      'path' => NULL,
      'database' => NULL,
    );

    $url = (object) array_map('urldecode', $url);

    $db_spec = array(
      'driver' => 'mysql',
      'username' => $url->user,
      'password' => $url->pass,
      'port' => $url->port,
      'host' => $url->host,
      // Remove leading / character from database names.
      'database' => substr($url->path, 1),
    );
  }

  foreach ($default_options as $name => $value) {
    if (!isset($options[$name])) {
      $options[$name] = $value;
    }
  }

  $settings = array(
    'parameters' => array(
      'profile' => $profile,
      'locale' => $options['locale'],
    ),
    'forms' => array(
      'install_settings_form' => array(
        'driver' => $db_spec['driver'],
        $db_spec['driver'] => $db_spec,
        'op' => 'Save and continue',
      ),
      'install_configure_form' => array(
        'site_name' => $options['site-name'],
        'site_mail' => $options['site-mail'],
        'account' => array(
          'name' => $options['account-name'],
          'mail' => $options['account-mail'],
          'pass' => array(
            'pass1' => $options['account-pass'],
            'pass2' => $options['account-pass'],
          ),
        ),
        'update_status_module' => array(
          1 => TRUE,
          2 => TRUE,
        ),
        'clean_url' => $options['clean-url'],
        'op' => 'Save and continue',
      ),
    ),
  );

  // Merge in the additional options.
  foreach ($additional_form_options as $key => $value) {
    $current = &$settings['forms'];
    foreach (explode('.', $key) as $param) {
      $current = &$current[$param];
    }
    $current = $value;
  }

  define('MAINTENANCE_MODE', 'install');
  require_once './core/includes/install.core.inc';

  try {
    render_text(array('value' => "Installing Backdrop. This may take a moment...", '#color' => 'bold_blue'));
    install_backdrop($settings);
    b_set_message(bt("Backdrop installed successfully."));
    b_set_message(bt('Username: ' . $default_options['account-name']));
    b_set_message(bt('Password: ' . $default_options['account-pass']));
    return TRUE;
  }
  catch (Exception $e) {
    echo $e->getMessage();
    b_set_message(bt("An error occurred. Output of installation attempt is as follows: !error", array('!error' => $e->getMessage())), 'error');
    return FALSE;
  }
}
