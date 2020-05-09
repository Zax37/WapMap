#ifndef SHR_GCN_TOOLTIP
#define SHR_GCN_TOOLTIP

#define SHR_TOOLTIP_DELAY 0.25f //in seconds

namespace SHR {
    class cTooltip {
    private:
        char *m_szTooltip;
        float m_fTooltipTimer;
        int m_iTooltipX, m_iTooltipY;
        bool m_bTooltipVisible;
        int m_iTooltipLC;

    protected:
        static cTooltip* currentlyDisplayedTooltip;
        void _RenderTooltip();

    public:
        cTooltip();

        ~cTooltip();

        void SetTooltip(const char *n);

        bool HasTooltip() { return (m_szTooltip && *m_szTooltip); };

        bool IsTooltipVisible() { return m_bTooltipVisible; };

        const char *GetTooltip() const { return m_szTooltip; };

        static void RenderTooltip();

        void UpdateTooltip(bool hasmouse);
    };
}

#endif
