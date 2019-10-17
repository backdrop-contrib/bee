<?php
/**
 * Implementation of hook_b_command().
 */
function themes_b_command() {
  $items['pm-enable'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => 'Enable one or more themes.',
    'arguments' => array(
      'extensions' => array(
        'description' => 'List of themes.',
        'multiple' => TRUE,
      ),
    ),
    'options' => array(
      'default' => bt('Set theme as default'),
      'admin' => bt('Set as admin theme'),
    ),
    'callback' => 'themes_b_enable_callback',
    'aliases' => array('en'),
  );
  $items['pm-disable'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => 'Disable one or more themes.',
    'arguments' => array(
      'extensions' => array(
        'description' => 'List of themes.',
        'multiple' => TRUE,
      ),
    ),
    'aliases' => array('dis'),
    'callback' => 'themes_b_disable_callback',
  );
  return $items;
}

/**
 * pm-enable callback. Enable theme.
 */
function themes_b_enable_callback($arguments, $options) {
  if(!isset($arguments['extensions'])){
    b_set_message('Please provide theme name(s)', 'error');
    return;
  }

  $clean_cache = FALSE;
  foreach($arguments['extensions'] as $project ) {
    if(_enable_theme($project)) {
      $clean_cache = TRUE;
    }
  }

  // If there is only one theme
  if(count($arguments['extensions']) == 1) {
    // And option --default
    if(isset($options['default'])) {
      // Set theme default.
      config_set('system.core', 'theme_default', $arguments['extensions'][0]);
    }
    // And option --admin
    if(isset($options['admin'])) {
      // Set admintheme default.
      config_set('system.core', 'admin_theme', $arguments['extensions'][0]);
    }
  }

  if($clean_cache) {
    backdrop_flush_all_caches();
  }
}

/**
 * Internal function to enable theme.
 */
function _enable_theme($project) {
  // Rescan for new themes.
  system_rebuild_theme_data();
  
  $query = db_select('system', 's')
      ->fields('s');
  $query->condition('name', $project);
  $query->condition('type', 'theme');
  $theme = $query->execute()->fetchAssoc();

  if(!$theme){
    return FALSE;
  }

  if($theme['status']) {
    b_set_message("Failed to enable theme " . $theme['name']. ": it is enabled already", 'error');
    return FALSE;
  }

  $info =  unserialize($theme['info']);
  $rows[] = array(
    array('data' => bt('Name'), '#color' => 'bold_blue'),
    $info['name'],
  );
  $rows[] = array(
    array('data' => bt('Description'), '#color' => 'bold_blue'),
    $info['description'],
  );
  $rows[] = array(
    array('data' => bt('Engine'), '#color' => 'bold_blue'),
    $info['engine'],
  );
  $rows[] = array(
    array('data' => bt('Version'), '#color' => 'bold_blue'),
    $info['version'],
  );

  $output[] = array(
    '#type' => 'table',
    'rows' => $rows,
    'delimiter' => ' : ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  );

  b_render($output);

  if (!b_confirm(bt('Do you want to enable?'))) {
    b_set_message(bt('Cancelled to enable !name', array('!name' => $theme['name'])));
    return;
  }

  theme_enable(array($theme['name']));
  b_set_message("Theme " . $theme['name'] . " enabled", 'success');
  return TRUE;
}

/**
 * pm-disable callback. Disable theme(s).
 */
function themes_b_disable_callback($arguments, $options) {

  if(!isset($arguments['extensions'])){
    b_set_message('Please provide theme name(s)', 'error');
    return;
  }

  $clean_cache = FALSE;
  foreach($arguments['extensions'] as $project ) {
    if(_disable_theme($project)) {
      $clean_cache = TRUE;
    }
  }

  if($clean_cache) {
    backdrop_flush_all_caches();
  }
}

/**
 * Internal function to disable theme.
 */
function _disable_theme($project) {
  $query = db_select('system', 's')
      ->fields('s');
  $query->condition('name', $project);
  $query->condition('type', 'theme');
  $theme = $query->execute()->fetchAssoc();

  if(!$theme){
    return FALSE;
  }

  if(!$theme['status']) {
    b_set_message("Failed to disable theme " . $theme['name']. ": it is disabled already", 'error');
    return FALSE;
  }

  $info =  unserialize($theme['info']);
  $rows[] = array(
    array('data' => bt('Name'), '#color' => 'bold_blue'),
    $info['name'],
  );
  $rows[] = array(
    array('data' => bt('Description'), '#color' => 'bold_blue'),
    $info['description'],
  );
  $rows[] = array(
    array('data' => bt('Engine'), '#color' => 'bold_blue'),
    $info['engine'],
  );
  $rows[] = array(
    array('data' => bt('Version'), '#color' => 'bold_blue'),
    $info['version'],
  );

  $output[] = array(
    '#type' => 'table',
    'rows' => $rows,
    'delimiter' => ' : ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  );

  b_render($output);

  if (!b_confirm(bt('Do you want to enable?'))) {
    b_set_message(bt('Cancelled to enable !name', array('!name' => $theme['name'])));
    return;
  }


  theme_disable(array($theme['name']));
  b_set_message("Theme " . $theme['name'] . " disabled", 'success');
  return TRUE;
}
