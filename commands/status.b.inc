<?php
  
  /**
 * Implementation of hook_b_command().
 */
function status_b_command() {
  $items = array();
  // We specify command callbacks here because the defaults would collide with
  // the drush cache api functions.
 $items['core-status'] = array(
    'description' => 'Provides a birds-eye view of the current Backdrop installation, if any.',
    'bootstrap' => DRUSH_BOOTSTRAP_MAX,
    'aliases' => array('status', 'st'),
    'examples' => array(
      'drush core-status version' => 'Show all status lines that contain version information.',
      'drush core-status --pipe' => 'A list key=value items separated by line breaks.',
      'drush core-status drush-version --pipe' => 'Emit just the drush version with no label.',
      'drush core-status config-active --pipe' => 'Emit just the active Config directory with no label.',
    ),
    'arguments' => array(
      'item' => 'Optional.  The status item line(s) to display.',
    ),
    'options' => array(
      'show-passwords' => 'Show database password.',
      'full' => 'Show all file paths and drush aliases in the report, even if there are a lot.',
      'project' => array(
        'description' => 'One or more projects that should be added to the path list',
        'example-value' => 'foo,bar',
      ),
    ),
    'outputformat' => array(
      'default' => 'key-value',
      'pipe-format' => 'json',
      'field-labels' => array('drupal-version' => 'Drupal version', 'uri' => 'Site URI', 'db-driver' => 'Database driver', 'db-host' => 'Database hostname', 'db-username' => 'Database username', 'db-password' => 'Database password', 'db-name' => 'Database name', 'db-status' => 'Database', 'bootstrap' => 'Drupal bootstrap', 'user' => 'Drupal user', 'theme' => 'Default theme', 'admin-theme' => 'Administration theme', 'php-bin' => 'PHP executable', 'php-conf' => 'PHP configuration', 'php-os' => 'PHP OS', 'drush-version' => 'Drush version', 'drush-conf' => 'Drush configuration', 'drush-alias-files' => 'Drush alias files', 'root' => 'Drupal root', 'site-path' => 'Site path', 'root' => 'Drupal root', 'site' => 'Site path', 'themes' => 'Themes path', 'modules' => 'Modules path', 'files' => 'File directory path', 'private' => 'Private file directory path', 'temp' => 'Temporary file directory path', 'config-active' => 'Active config directory path', 'config-staging' => 'Staging config directory path', 'files-path' => 'File directory path', 'temp-path' => 'Temporary file directory path', '%paths' => 'Other paths'),
      'formatted-filter' => '_drush_core_status_format_table_data',
      'private-fields' => 'db-password',
      'simplify-single' => TRUE,
      'table-metadata' => array(
        'list-separator' => ' ',
      ),
      'output-data-type' => 'format-list',
    ),
    'topics' => array('docs-readme'),
  );

  return $items;
}