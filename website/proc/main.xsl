<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet
  version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  >

  <!--
  =============================================================================
  Output style
  =============================================================================
  -->

  <xsl:output
    method="xml"
    version="1.0"
    encoding="utf-8"
    media-type="text/xml"
    indent="yes"
    doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
    doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
    />

  <!--
  =============================================================================
  Variables and parameters
  =============================================================================
  -->

  <xsl:variable name="lang">
    <xsl:value-of select="/page/@lang"/>
  </xsl:variable>

  <xsl:variable
    name="shared"
    select="document('../src/shared.xml')/shared"
  />

  <xsl:variable name="nbsections">
    <xsl:value-of select="count(/page/section)"/>
  </xsl:variable>

  <!--
  =============================================================================
  Main template
  =============================================================================
  -->

  <xsl:template match="page">
    <html lang="{$lang}" xml:lang="{@lang}">
      <xsl:call-template name="htmlhead"/>
      <body>
        <!-- delegate top elements creation -->
        <xsl:call-template name="bodyhead"/>

        <!-- content -->
        <div id="content">
          <!-- make a toc if more thant one section -->
<!--          <xsl:if test="count(section) > 1">-->
          <xsl:if test="$nbsections > 1">
            <div id="toc">
              <ul>
                <xsl:apply-templates select="section" mode="toc"/>
              </ul>
            </div>
          </xsl:if>
          <xsl:apply-templates select="section"/>
        </div>

        <!-- menu generation -->
        <xsl:call-template name="menu" mode="menu"/>

        <!-- delegate bottom elements creation -->
        <xsl:call-template name="bodyfoot"/>

      </body>
    </html>
  </xsl:template>

  <!--
  =============================================================================
  First level templates
  =============================================================================
  -->

  <!-- HTML head element with its content -->
  <xsl:template name="htmlhead">
    <head>
      <title><xsl:value-of select="normalize-space(head/title)"/></title>
      <xsl:for-each select="$shared/htmlhead[@lang=$lang or not(@lang)]">
        <xsl:apply-templates/>
      </xsl:for-each>
    </head>
  </xsl:template>

  <!-- top of the page, aka the body-header -->
  <xsl:template name="bodyhead">
    <div id="head">
      <xsl:for-each select="$shared/bodyhead[@lang=$lang or not(@lang)]">
        <xsl:apply-templates/>
      </xsl:for-each>
      <h1><xsl:value-of select="head/title"/></h1>
    </div>
  </xsl:template>

  <!-- bottom of the page, aka the body-footer -->
  <xsl:template name="bodyfoot">
    <div id="foot">
      <xsl:for-each select="$shared/bodyfoot[@lang=$lang or not(@lang)]">
        <xsl:apply-templates/>
      </xsl:for-each>
    </div>
  </xsl:template>

  <!-- table of content -->
  <xsl:template match="section" mode="toc">
    <!-- sections with identifier -->
    <xsl:if test="@id">
      <li>
        <a href="#{@id}">
          <xsl:value-of select="@title"/>
        </a>

        <!-- nested sections -->
        <xsl:if test="section">
          <ul>
            <!-- recursive call -->
            <xsl:apply-templates select="section" mode="toc"/>
          </ul>
        </xsl:if>
      </li>
    </xsl:if>

    <!-- sections without identifier -->
    <xsl:if test="not(@id)">
      <li>
        <xsl:value-of select="@title"/>

        <!-- nested sections -->
        <xsl:if test="section">
          <ul>
            <!-- recursive call -->
            <xsl:apply-templates select="section" mode="toc"/>
          </ul>
        </xsl:if>
      </li>
    </xsl:if>
  </xsl:template>

  <xsl:template match="section">
    <div class="section">
      <!-- section head -->
      <xsl:if test="$nbsections > 1">
        <h2>
          <xsl:if test="@id">
            <a id="{@id}" name="{@id}"/>
          </xsl:if>
          <xsl:value-of select="@title"/>
        </h2>
      </xsl:if>
      <!-- section content -->
      <!-- special case for 'features' (include status.xml) -->
      <!-- special case for 'news' (include newsdb.xml) -->
      <xsl:choose>
        <xsl:when test="/page/@name='features' and @id='status'">
          <xsl:call-template name='status'/>
        </xsl:when>
        <xsl:when test="/page/@name='news' and @id='news'">
          <xsl:call-template name='news'/>
        </xsl:when>
      </xsl:choose>
      <xsl:apply-templates/>
    </div>
  </xsl:template>

  <xsl:template match="section/section">
    <!-- section head -->
    <h3>
      <xsl:if test="@id">
        <a id="{@id}" name="{@id}"/>
      </xsl:if>
      <xsl:value-of select="@title"/>
    </h3>
    <!-- section content -->
    <xsl:apply-templates/>
  </xsl:template>
  
  <xsl:template match="section/section/section">
    <!-- section head -->
    <h4>
      <xsl:if test="@id">
        <a id="{@id}" name="{@id}"/>
      </xsl:if>
      <xsl:value-of select="@title"/>
    </h4>
    <!-- section content -->
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="section/section/section/section">
    <!-- section head -->
    <h5>
      <xsl:if test="@id">
        <a id="{@id}" name="{@id}"/>
      </xsl:if>
      <xsl:value-of select="@title"/>
    </h5>
    <!-- section content -->
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="list">
    <xsl:choose>
      <xsl:when test="@type='authors'">
        <xsl:call-template name='authors'/>
      </xsl:when>
    </xsl:choose>
  </xsl:template>
  <!--
  =============================================================================
  Second level templates
  =============================================================================
  -->

  <!-- menu -->
  <xsl:template name="menu" mode="menu">
    <div id="menu">
      <ul>
        <xsl:variable name="menu" select="document('../src/menu.xml')/menu"/>
        <xsl:for-each select="$menu/item">
          <li class="menuitem">
            <xsl:element name="a">
<!--              <xsl:attribute name="href"><xsl:value-of select="@name"/>.<xsl:value-of select="$lang"/>.html</xsl:attribute>-->
              <xsl:attribute name="href"><xsl:value-of select="@name"/></xsl:attribute>
              <xsl:value-of select="./text[@lang=$lang]"/>
            </xsl:element>
          </li>
        </xsl:for-each>
      </ul>
    </div>
  </xsl:template>

  <!-- status in features page -->
  <xsl:template name="status">
    <xsl:variable name="status" select="document('../src/status.xml')/status"/>
    <xsl:variable name="caption" select="$status/title/text[@lang=$lang]"/>
    <xsl:element name="table">
      <xsl:attribute name="summary">
        <xsl:value-of select="$caption"/>
      </xsl:attribute>
      <xsl:element name="caption"><xsl:value-of select="$caption"/></xsl:element>
      <thead>
        <tr>
          <xsl:for-each select="$status/header/col">
            <th><xsl:value-of select="text[@lang=$lang]"/></th>
          </xsl:for-each>
        </tr>
      </thead>
      <tbody>
        <xsl:for-each select="$status/item">
          <tr>
            <td><xsl:value-of select="title/text[@lang=$lang]"/></td>
            <td><xsl:value-of select="state"/></td>
            <td><xsl:value-of select="comment/text[@lang=$lang]"/></td>
          </tr>
        </xsl:for-each>
      </tbody>
    </xsl:element>
  </xsl:template>

  <!-- authors in about page -->
  <xsl:template name='authors'>
    <xsl:variable name='authors' select="document('../src/authors.xml')/authors"/>
    <ul>
      <xsl:for-each select="$authors/author">
      <li>
      <xsl:element name="a">
        <xsl:attribute name="href"><![CDATA[mailto:]]><xsl:value-of select="email"/></xsl:attribute>
        <xsl:value-of select="name"/>
      </xsl:element>
      <xsl:if test="nick"> (<xsl:value-of select="nick"/>)</xsl:if>
      <xsl:if test="role/text[@lang=$lang]">
        - <xsl:value-of select="role/text[@lang=$lang]"/>
      </xsl:if>
      </li>
      </xsl:for-each>
    </ul>
  </xsl:template>

  <!-- news in news page -->
  <xsl:template name='news'>
    <xsl:variable name='news' select="document('../src/newsdb.xml')/news"/>
    <xsl:if test="count($news/item[itemlang/@lang=$lang])">
      <xsl:for-each select="$news/item[itemlang/@lang=$lang]">
        <xsl:variable name="newsitem" select="itemlang[@lang=$lang]"/>
        <div class="newsitem">
          <div class="newstitle">
            <xsl:variable name="itemdate" select="@date"/>
            <xsl:choose>
              <xsl:when test="$lang='fr'">
                <xsl:value-of select="concat(substring($itemdate, 7, 2), '/', substring($itemdate, 5, 2), '/', substring($itemdate, 1, 4))"/>
              </xsl:when>
              <xsl:when test="$lang='en'">
                <xsl:value-of select="concat(substring($itemdate, 5, 2), '/', substring($itemdate, 7, 2), '/', substring($itemdate, 1, 4))"/>
              </xsl:when>
            </xsl:choose>
            : <xsl:value-of select="$newsitem/title"/>
          </div>
          <div class="newstext">
            <xsl:value-of select="$newsitem/text"/>
          </div>
        </div>
      </xsl:for-each>
    </xsl:if>
  </xsl:template>

  <!--
  =============================================================================
  Default templates
  =============================================================================
  -->
  <xsl:template match="@*|*|text()">
    <xsl:copy>
      <xsl:apply-templates select="@*|*|text()"/>
    </xsl:copy>
  </xsl:template>

</xsl:stylesheet>
