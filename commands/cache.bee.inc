<?php
/**
 * @file
 * Command(s) for working with Backdrop's cache.
 */

/**
 * Implements hook_bee_command().
 */
function cache_bee_command() {
  return array(
    'cache-clear' => array(
      'description' => bt('Clear a specific cache, or all Backdrop caches.'),
      'callback' => 'cache_clear_bee_command',
      'group' => 'miscellaneous',
      'arguments' => array(
        'cache' => bt('The name of the cache to clear (not case sensitive). Leave blank to see a list of available caches.'),
      ),
      'optional_arguments' => array('cache'),
      'aliases' => array('cc'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee cache-clear menu' => bt('Clear the menu cache.'),
        'bee cache-clear all' => bt('Clear all caches.'),
        'bee cache-clear css_js' => bt('Clear the CSS & JS cache.'),
        'bee cache-clear' => bt('Select the cache to clear from a list of available ones.'),
      ),
    ),
  );
}

/**
 * Command callback: Clear a specific cache, or all Backdrop caches.
 */
function cache_clear_bee_command($arguments, $options) {
  // Get a list of all caches.
  $cache_list = array(
    'all' => bt('All'),
    'core' => bt('Core (page, admin bar, etc.)'),
    'css_js' => bt('CSS & JS [css_js]'),
    'entity' => bt('Entity'),
    'layout' => bt('Layout'),
    'menu' => bt('Menu'),
    'module' => bt('Module'),
    'theme' => bt('Theme'),
    'token' => bt('Token'),
    'update' => bt('Update'),
  );

  // Get the cache to clear.
  $cache = isset($arguments['cache']) ? strtolower($arguments['cache']) : NULL;

  // Check the cache.
  if (!is_null($cache) && !isset($cache_list[$cache])) {
    bee_instant_message(bt("'!cache' is not a valid cache to clear.", array(
      "!cache" => $cache,
    )) . "\n", 'warning'); // Add additional linebreak before select list.
    $cache = NULL;
  }

  // If either no cache was entered or an invalid cache was entered, offer a
  // choice of caches to clear.
  if (is_null($cache)) {
    $cache = bee_choice($cache_list, bt('Select a cache to clear:'), 'all');
  }

  // Clear the cache.
  switch ($cache) {
    case 'all':
      backdrop_flush_all_caches();
      break;

    case 'core':
      $core = array('cache', 'path', 'filter', 'bootstrap', 'token', 'page');
      $cache_bins = array_merge(module_invoke_all('flush_caches'), $core);
      foreach ($cache_bins as $bin) {
        cache($bin)->flush();
      }
      break;

    case 'css_js':
      _backdrop_flush_css_js();
      backdrop_clear_css_cache();
      backdrop_clear_js_cache();
      cache_flush('cache_page');
      break;

    case 'entity':
      entity_info_cache_clear();
      break;

    case 'layout':
      layout_reset_caches();
      break;

    case 'menu':
      menu_rebuild();
      break;

    case 'module':
      system_rebuild_module_data();
      _system_update_bootstrap_status();
      break;

    case 'theme':
      system_rebuild_theme_data();
      backdrop_theme_rebuild();
      break;

    case 'token':
      token_cache_clear();
      break;

    case 'update':
      _update_cache_clear();
      break;
  }

  // Display success message.
  bee_message(bt('Cache(s) cleared: !cache', array(
    '!cache' => $cache_list[$cache],
  )), 'success');
}
