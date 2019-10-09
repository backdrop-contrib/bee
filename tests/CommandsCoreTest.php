<?php
/**
 * @file
 * PHPUnit tests for Backdrop Console Core commands.
 */

use PHPUnit\Framework\TestCase;

class CommandsCoreTest extends TestCase {

  /**
   * Check that the Cache Clear command works.
   */
  public function testCacheClear() {
    $output_all = shell_exec('b cc all');
    $output_menu = shell_exec('b cc menu');
    $output_update = shell_exec('b cc update');

    $this->assertStringContainsString('All cache cleared', $output_all);
    $this->assertStringContainsString('Menu cache cleared', $output_menu);
    $this->assertStringContainsString('Update data cache cleared', $output_update);
  }

  /**
   * Check that the Config List command works.
   */
  public function testConfigList() {
    $output = shell_exec('b config-list');

    $this->assertStringContainsString('system.core', $output);
    $this->assertStringContainsString('System core settings', $output);
  }

  /**
   * Check that the Config Get command works.
   */
  public function testConfigGet() {
    $output = shell_exec('b config-get system.core');

    $this->assertStringContainsString('site_favicon_path', $output);
    $this->assertStringContainsString('core/misc/favicon.ico', $output);
  }

  /**
   * Check that the Config Set command works.
   */
  public function testConfigSet() {
    $config = 'system.core';
    $setting = 'site_frontpage';

    exec("b config-get $config", $output_get_old);
    $output_set = shell_exec("b config-set $config $setting about");
    exec("b config-get $config", $output_get_new);

    // Make sure the old homepage is 'home'.
    foreach ($output_get_old as $output) {
      if (strpos($output, $setting) !== FALSE) {
        $this->assertStringContainsString('home', $output);
      }
    }

    $this->assertStringContainsString('Config updated', $output_set);

    // Make sure the new homepage is 'about'.
    foreach ($output_get_new as $output) {
      if (strpos($output, $setting) !== FALSE) {
        $this->assertStringContainsString('about', $output);
      }
    }
  }

  /**
   * Check that the Cron command works.
   */
  public function testCron() {
    $output = shell_exec('b cron');

    $this->assertStringContainsString('Cron processed', $output);
  }

}

