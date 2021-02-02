<?php
/**
 * @file
 * Command(s) for installing Backdrop.
 */

/**
 * Implements hook_b_command().
 */
function install_b_command() {
  return array(
    'install' => array(
      'description' => bt('Install Backdrop and setup a new site.'),
      'callback' => 'install_b_callback',
      'options' => array(
        'db' => array(
          'description' => bt('A database connection string of the form: `mysql://user:pass@localhost/database_name`. See https://api.backdropcms.org/database-configuration for more details.'),
          'value' => bt('String'),
        ),
        'username' => array(
          'description' => bt('The username for the primary administrator account. Defaults to `admin`.'),
          'value' => bt('Username'),
        ),
        'password' => array(
          'description' => bt('The password for the primary administrator account. Defaults to a randomly-generated string.'),
          'value' => bt('Password'),
        ),
        'email' => array(
          'description' => bt('The email address for the primary administrator account. Defaults to `admin@example.com`.'),
          'value' => bt('Email'),
        ),
        'site-name' => array(
          'description' => bt('The name of the site. Defaults to `My Backdrop Site`.'),
          'value' => bt('Name'),
        ),
        'site-mail' => array(
          'description' => bt("The 'From' address used in automated site emails. Defaults to `admin@example.com`."),
          'value' => bt('Email'),
        ),
        'profile' => array(
          'description' => bt('The machine-name of the installation profile to use. Defaults to `standard`.'),
          'value' => bt('Profile'),
        ),
        'langcode' => array(
          'description' => bt('The short code of the default site language. Language files must already be present. Defaults to `en`.'),
          'value' => bt('Language'),
        ),
        'db-prefix' => array(
          'description' => bt('The table prefix to use for this site in the database. Defaults to no prefix.'),
          'value' => bt('Prefix'),
        ),
        'no-clean-urls' => array(
          'description' => bt('Disable clean URLs.'),
        ),
        'auto' => array(
          'description' => bt('Perform an automatic (i.e. non-interactive) installation. Any options not explicitly provided to the command will use default values, except the database connection string which will always prompt when not provided.'),
        ),
      ),
      'aliases' => array('si', 'site-install'),
      'examples' => array(
        'b install' => bt('Install Backdrop in interactive mode, providing information when prompted.'),
        'b install --db=mysql://admin:P@ssw0rd!@localhost/backdrop --auto' => bt('Install Backdrop automatically using the provided database credentials, and default settings for everything else.'),
        'b install --db=mysql://admin:P@ssw0rd!@localhost/backdrop --username=Root --password=N0tS3cur3 --email=root@mydomain.com --site-name="My awesome site!"' => bt('Install Backdrop using the given options, and be prompted for the rest.'),
      ),
    ),
  );
}

/**
 * Command callback: Install Backdrop and setup a new site.
 */
function install_b_callback($arguments, $options) {
  global $backdrop_root;
  $prompt = empty($options['auto']);

  // Get database connection string.
  if (!empty($options['db'])) {
    $db = $options['db'];
  }
  else {
    $db_name = b_check_plain(b_input(bt('Database name'), '', TRUE));
    $db_username = b_check_plain(b_input(bt('Database username'), '', TRUE));
    $db_password = b_check_plain(b_input(bt('Database password'), '', TRUE));
    $db_host = b_check_plain(b_input(bt('Database host'), 'localhost', TRUE));
    $db = "mysql://$db_username:$db_password@$db_host/$db_name";
  }
  $db = "--db-url='$db'";

  // Get database prefix.
  if (!empty($options['db-prefix'])) {
    $db_prefix = $options['db-prefix'];
  }
  else {
    $db_prefix = ($prompt) ? b_check_plain(b_input(bt('Database prefix'), '')) : '';
  }
  $db_prefix = "--db-prefix='$db_prefix'";

  // Get admin username.
  if (!empty($options['username'])) {
    $username = $options['username'];
  }
  else {
    $username = ($prompt) ? b_check_plain(b_input(bt('Admin username'), 'admin', TRUE)) : 'admin';
  }
  $username = "--account-name='$username'";

  // Get admin password.
  $random = substr(md5(rand()), 0, 20);
  if (!empty($options['password'])) {
    $password = $options['password'];
  }
  else {
    $password = ($prompt) ? b_check_plain(b_input(bt('Admin password'), $random, TRUE)) : $random;
  }
  $show_password = ($password == $random) ? TRUE : FALSE;
  $password = "--account-pass='$password'";

  // Get admin email.
  if (!empty($options['email'])) {
    $email = $options['email'];
  }
  else {
    $email = ($prompt) ? b_check_plain(b_input(bt('Admin email'), 'admin@example.com', TRUE)) : 'admin@example.com';
  }
  $email = "--account-name='$email'";

  // Get site name.
  if (!empty($options['site-name'])) {
    $site_name = $options['site-name'];
  }
  else {
    $site_name = ($prompt) ? b_check_plain(b_input(bt('Site name'), 'My Backdrop Site', TRUE)) : 'My Backdrop Site';
  }
  $site_name = "--site-name='$site_name'";

  // Get site mail.
  if (!empty($options['site-mail'])) {
    $site_mail = $options['site-mail'];
  }
  else {
    $site_mail = ($prompt) ? b_check_plain(b_input(bt('Site mail'), 'admin@example.com', TRUE)) : 'admin@example.com';
  }
  $site_mail = "--site-mail='$site_mail'";

  // Get language.
  if (!empty($options['langcode'])) {
    $langcode = $options['langcode'];
  }
  else {
    $langcode = ($prompt) ? b_check_plain(b_input(bt('Language code'), 'en', TRUE)) : 'en';
  }
  $langcode = "--langcode='$langcode'";

  // Get clean URLs.
  $clean_urls = (!empty($options['no-clean-urls'])) ? 0 : 1;
  $clean_urls = "--clean-url=$clean_urls";

  // Get profile.
  if (!empty($options['profile'])) {
    $profile = $options['profile'];
  }
  else {
    $profile = ($prompt) ? b_check_plain(b_input(bt('Install profile'), 'standard', TRUE)) : 'standard';
  }

  // Run the install script.
  $result = exec("cd $backdrop_root && core/scripts/install.sh $db $db_prefix $username $password $email $site_name $site_mail $langcode $clean_urls $profile");
  b_render_text(array('value' => $result));

  // Show the randomly-generated password.
  if (strpos($result, 'successfully') !== FALSE && $show_password) {
    b_render_text(array('value' => bt('The randomly-generated admin password is: ' . $random)), FALSE);
  }
}

/**
 * Encodes special characters in a plain-text string.
 *
 * @see check_plain()
 *
 * Copied here so it can be used while the Backdrop site is being installed.
 */
function b_check_plain($text) {
  return htmlspecialchars($text, ENT_QUOTES, 'UTF-8');
}
