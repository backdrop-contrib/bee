<?php
/**
 * Implements of hook_b_command().
 */
function cache_b_command() {
  $items = array();

  $items['cache-clear'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => 'Clear a specific cache, or all Backdrop caches.',
    'arguments' => array(
      'type' => 'The particular cache to clear. Omit this argument to choose from available caches.',
    ),
    'callback' => 'cache_command_clear',
    'aliases' => array('cc'),
  );
  return $items;
}

/**
 * @param array $arguments
 *  cache clear and optional type
 * @param $options
 *  todo: are we using this?
 * @return array
 *  Success message to user.
 */
function cache_command_clear($arguments, $options) {
  $caches = array(
    'all' => t('All'),
    'assets' => t('CSS and JavaScript'),
    'cache' => t('Page and else'),
    'layout' => t('Layout'),
    'menu' => t('Menu'),
    'theme' => t('Theme registry'),
    'token' => t('Token registry'),
    'admin_bar' => t('Administration bar'),
    'update' => t('Update data'),
  );
  if (!isset($arguments['type'])) {
    if(!$choice = b_choice($caches, bt('Enter a number to choose which cache to clear'))) {
      b_set_message(bt('Cancelled.'));
      return FALSE;
    }
  }
  else{
    $choice = $arguments['type'];
  }

  switch($choice) {
    case 'all':
        backdrop_flush_all_caches();
      break;
    case 'update':
        _update_cache_clear();
      break;
     case 'admin_bar':
    cache('admin_bar')->flush();
    break;

  case 'menu':
    menu_rebuild();
    break;

  case 'cache':
    // Don't clear cache_form - in-progress form submissions may break.
    // Ordered so clearing the page cache will always be the last action.
    // @see backdrop_flush_all_caches()
    $core = array('cache', 'path', 'filter', 'bootstrap', 'token', 'page');
    $cache_bins = array_merge(module_invoke_all('flush_caches'), $core);
    foreach ($cache_bins as $bin) {
      cache($bin)->flush();
    }
    break;

  case 'assets':
    // Change query-strings on css/js files to enforce reload for all users.
    _backdrop_flush_css_js();

    backdrop_clear_css_cache();
    backdrop_clear_js_cache();

    // Clear the page cache, since cached HTML pages might link to old CSS and
    // JS aggregates.
    cache_clear_all('*', 'cache_page', TRUE);
    break;

  case 'theme':
    system_rebuild_theme_data();
    backdrop_theme_rebuild();
    break;

  case 'token':
    token_cache_clear();
    break;

  case 'layout':
    layout_reset_caches();
    break;
  default: 
    b_set_message(bt('Unsupported option !name!', array('!name' => $choice)), 'error');
    $rows = array();
    foreach($caches as $key => $value) {
      $rows[] = array(
        array( 'data' => $key, '#color' => 'bold_blue'),
        $value,
      );
    }
    return array(
      '#type' => 'table',
      'rows' => $rows,
      'delimiter' => ' : ',
      'delimiter_left' => ' ',
      'delimiter_right' => ' ',
    );
  }

  b_set_message(bt("!type cache cleared.", array('!type' => $caches[$choice])), 'success');
}
