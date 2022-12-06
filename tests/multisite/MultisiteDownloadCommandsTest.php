<?php
/**
 * @file
 * PHPUnit tests for Bee multisite Download commands.
 */

use PHPUnit\Framework\TestCase;
/**
 * Test Bee's command for downloading modules, themes and layouts in multisite.
 */
class MultisiteDownloadCommandsTest extends TestCase {

  /**
   * Make sure that the download command works for multisites.
   */
  public function test_download_command_works() {
    // Root directory, no site specified.
    $output_root = shell_exec('bee download --hide-progress simplify');
    $this->assertStringContainsString("'simplify' was downloaded into '/app/multisite/modules/simplify'.", (string) $output_root);
    $this->assertTrue(file_exists('/app/multisite/modules/simplify/simplify.info'));

    // Root directory, site specified.
    $output_root_site = shell_exec('bee download --site=multi_one --hide-progress lumi');
    $this->assertStringContainsString("'lumi' was downloaded into '/app/multisite/sites/multi_one/themes/lumi'.", (string) $output_root_site);
    $this->assertTrue(file_exists('/app/multisite/sites/multi_one/themes/lumi/lumi.info'));

    // Site directory.
    $output_site = shell_exec('cd sites/multi_two && bee download --hide-progress bamboo');
    $this->assertStringContainsString("'bamboo' was downloaded into '/app/multisite/sites/multi_two/layouts/bamboo'.", (string) $output_site);
    $this->assertTrue(file_exists('/app/multisite/sites/multi_two/layouts/bamboo/bamboo.info'));

    // Cleanup downloads.
    exec('rm -fr /app/multisite/modules/simplify /app/multisite/sites/multi_one/themes /app/multisite/sites/multi_two/layouts');
  }

}
