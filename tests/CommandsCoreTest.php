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
    $this->assertStringContainsString('All cache cleared', $output_all);

    $output_menu = shell_exec('b cc menu');
    $this->assertStringContainsString('Menu cache cleared', $output_menu);

    $output_update = shell_exec('b cc update');
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

    // Make sure the old homepage is 'home'.
    exec("b config-get $config", $output_get_old);
    foreach ($output_get_old as $output) {
      if (strpos($output, $setting) !== FALSE) {
        $this->assertStringContainsString('home', $output);
      }
    }

    $output_set = shell_exec("b config-set $config $setting about");
    $this->assertStringContainsString('Config updated', $output_set);

    // Make sure the new homepage is 'about'.
    exec("b config-get $config", $output_get_new);
    foreach ($output_get_new as $output) {
      if (strpos($output, $setting) !== FALSE) {
        $this->assertStringContainsString('about', $output);
      }
    }

    // Set frontpage back to 'home' for future tests.
    exec("b config-set $config $setting home");
  }

  /**
   * Check that the Cron command works.
   */
  public function testCron() {
    $output = shell_exec('b cron');
    $this->assertStringContainsString('Cron processed', $output);
  }

  /**
   * Check that the Watchdog Show command works.
   */
  public function testWatchdogShow() {
    $output_all = shell_exec('b ws');
    $this->assertStringContainsString('Date', $output_all);
    $this->assertStringContainsString('Message', $output_all);

    $output_one = shell_exec('b ws 1');
    $this->assertStringContainsString('ID', $output_one);
    $this->assertStringContainsString('Severity', $output_one);

    exec('b ws --count=2', $output_count);
    $this->assertEquals(3, count($output_count));

    $output_severity = shell_exec('b ws --severity=info');
    $this->assertStringNotContainsString('notice', $output_severity);

    $output_type = shell_exec('b ws --type=cron');
    $this->assertStringNotContainsString('system', $output_type);
  }

  /**
   * Check that the Site Install command works.
   *
   * TODO: @see https://github.com/backdrop-contrib/b/issues/53
   */
  public function testSiteInstall() {
    // TODO: Fix!
    $this->markTestSkipped('This test is still under development...');

    // Since this test uses environment-specific information, it is only run on
    // Travis CI.
    print_r('Travis: ' . getenv('TRAVIS'));
    if (getenv('TRAVIS') !== TRUE) {
      $this->markTestSkipped('This test is only run on Travis CI.');
    }

    $path = getenv('HOME') . '/site_install';
    $db = 'site_install';

    // Download Backdrop and create database.
    exec("git clone https://github.com/backdrop/backdrop.git $path");
    exec("mysql -e 'CREATE DATABASE $db'");

    $output = shell_exec("b si --root=$path --db_url=mysql://travis:@127.0.0.1/$db");
    print_r($output);
  }

  /**
   * Check that the Core Status command works.
   */
  public function testCoreStatus() {
    $asserted = FALSE;

    exec('b st', $output_default);
    foreach ($output_default as $output) {
      if (strpos($output, 'Backdrop root') !== FALSE) {
        $this->assertStringContainsString(getcwd(), $output);
        $asserted = TRUE;
      }
      if (strpos($output, 'Database name') !== FALSE) {
        $this->assertStringContainsString('backdrop', $output);
        $asserted = TRUE;
      }
    }
    $this->assertTrue($asserted, 'Expected output not found.');
  }

  /**
   * Check that the Test callback works.
   */
  public function testTestCallback() {
    $output = shell_exec('b st');
    $this->assertStringContainsString('Backdrop CMS Installation detected', $output);
  }

  /**
   * Check that the Update DB Status command works.
   */
  public function testUpdateDbStatus() {
    // Install an old version of Devel.
    exec('cd modules && wget -q https://github.com/backdrop-contrib/devel/releases/download/1.x-1.5.5/devel.zip');
    exec('cd modules && unzip devel.zip && rm devel.zip');
    exec('b en --y devel');

    // Check there are no DB updates.
    $output_clean = shell_exec('b updbst');
    $this->assertStringContainsString('No database updates required', $output_clean);

    // Update Devel to a newer version.
    exec('cd modules && rm -rf devel && wget -q https://github.com/backdrop-contrib/devel/releases/download/1.x-1.6.0/devel.zip');
    exec('cd modules && unzip devel.zip && rm devel.zip');

    // Check for DB updates.
    $output_updates = shell_exec('b updbst');
    $this->assertStringContainsString('Remove option for Krumo skin', $output_updates);
  }

  /**
   * Check that the Update DB command works.
   */
  public function testUpdateDb() {
    // Perform the updates from the previous test.
    $output = shell_exec('b updb --y');
    $this->assertStringContainsString('Remove option for Krumo skin', $output);
    $this->assertStringContainsString('Do you wish to run all pending updates', $output);
    $this->assertStringContainsString('All updates processed', $output);

    // Uninstall Devel for future tests.
    exec('b dis --y devel');
    exec('b pmu --y devel');
    exec('rm -rf modules/devel');
  }

}

