<?php
/**
 * @file
 * PHPUnit tests for Bee Download commands.
 */

use PHPUnit\Framework\TestCase;
/**
 * Test Bee's command for downloading modules, themes and layouts.
 */
class DownloadCommandsTest extends TestCase {

  /**
   * Make sure that the download command works.
   */
  public function test_download_command_works() {
    // Single module.
    $output_single = shell_exec('bee download --hide-progress simplify');
    $this->assertStringContainsString("'simplify' was downloaded into '/app/backdrop/modules/simplify'.", (string) $output_single);
    $this->assertTrue(file_exists('/app/backdrop/modules/simplify/simplify.info'));

    // Multiple projects (theme and layout).
    $output_multiple = shell_exec('bee download --hide-progress lumi bamboo');
    $this->assertStringContainsString("'lumi' was downloaded into '/app/backdrop/themes/lumi'.", (string) $output_multiple);
    $this->assertTrue(file_exists('/app/backdrop/themes/lumi/lumi.info'));
    $this->assertStringContainsString("'bamboo' was downloaded into '/app/backdrop/layouts/bamboo'.", (string) $output_multiple);
    $this->assertTrue(file_exists('/app/backdrop/layouts/bamboo/bamboo.info'));

    // Cleanup downloads.
    exec('rm -fr /app/backdrop/modules/simplify /app/backdrop/themes/lumi /app/backdrop/layouts/bamboo');
  }

  /**
   * Make sure that the download-core command works.
   */
  public function test_download_core_command_works() {
    // Download to current directory.
    $output_current = shell_exec('mkdir /app/current && cd /app/current && bee download-core --hide-progress');
    $this->assertStringContainsString("Backdrop was downloaded into '/app/current'.", (string) $output_current);
    $this->assertTrue(file_exists('/app/current/index.php'));

    // Download to specified directory.
    $output_directory = shell_exec('bee download-core --hide-progress /app/directory');
    $this->assertStringContainsString("Backdrop was downloaded into '/app/directory'.", (string) $output_directory);
    $this->assertTrue(file_exists('/app/directory/index.php'));

    // Cleanup downloads.
    exec('rm -fr /app/current /app/directory');
  }

}
